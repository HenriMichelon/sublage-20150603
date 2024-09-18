package sublage.ant;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;
import java.util.Set;
import org.apache.tools.ant.BuildException;
import sublage.lang.ReservedWord;
import sublage.lexer.LexicalAnalyzer;
import sublage.lexer.LexicalSymbol;
import sublage.lexer.LexicalToken;

public class CompileTask extends AbstractTask {

    private String srcdir = "sources";
    private String destdir = "binaries";
    private String sublagec = "sublagec";
    private String debug;

    public String getDebug() {
        return debug;
    }

    public void setDebug(String debug) {
        this.debug = debug;
    }

    public void setSublagec(String sublagec) {
        this.sublagec = sublagec;
    }

    public void setSrcdir(String srcdir) {
        this.srcdir = srcdir;
    }

    public void setDestdir(String destdir) {
        this.destdir = destdir;
    }

    private void addTree(File file, Collection<File> all) {
        File[] children = file.listFiles();
        if (children != null) {
            for (File child : children) {
                if (child.isDirectory()) {
                    addTree(child, all);
                } else if (child.getName().endsWith(".source")) {
                    all.add(child);
                }
            }
        }
    }

    private void findDependencies(File source, Collection<File> depends,
            Set<File> scanned) throws IOException,
            LexicalAnalyzer.SyntaxError {
        if (scanned.contains(source)) {
            return;
        }
        LexicalAnalyzer lexer = new LexicalAnalyzer(new FileInputStream(source), true);
        LexicalToken lt;
        while ((lt = lexer.nextToken()) != null) {
            if (lt.getSymbol() == LexicalSymbol.EOF) {
                break;
            }
            if (lt.getSymbol() == LexicalSymbol.RESERVEDWORD) {
                LexicalToken.ReservedWord word = (LexicalToken.ReservedWord) lt;
                if (word.getValue() == ReservedWord.IMPORT) {
                    while ((lt = lexer.nextToken()) != null) {
                        if (lt.getSymbol() == LexicalSymbol.SPACE) {
                            continue;
                        }
                        break;
                    }
                    if ((lt != null) && (lt.getSymbol() == LexicalSymbol.IDENTIFIER)) {
                        LexicalToken.Identifier ident = (LexicalToken.Identifier) lt;
                        String path = source.getParent() + File.separator
                                + ident.getValue() + ".library.source";
                        File depend = new File(path);
                        if (depend.isFile()) {
                            findDependencies(depend, depends, scanned);
                            if (!depends.contains(depend)) {
                                depends.add(depend);
                                System.out.println("fd add:  " + depend);
                            }
                        }
                    }
                }
            }
        }
        depends.add(source);
        scanned.add(source);
    }

    private List<File> sortedDependencies(Collection<File> all) {
        List<File> depends = new ArrayList<File>();
        Set<File> scanned = new HashSet<File>();
        for (File source : all) {
            try {
                findDependencies(source, depends, scanned);
            } catch (LexicalAnalyzer.SyntaxError ex) {
            } catch (IOException ex) {
                throw new BuildException("Error reading " + source
                        + " when searhing for depencies");
            }
        }
        return depends;
    }

    private String getBinary(File file, File dst) {
        String dest = dst + File.separator;
        if (file.getName().endsWith(".library.source")) {
            dest += file.getName().replaceAll("\\.library\\.source$", ".library");
        } else {
            dest += file.getName().replaceAll("\\.source$", ".binary");
        }
        return dest;
    }

    private List<File> onlyModified(Collection<File> all, File dst) {
        List<File> result = new ArrayList<File>();
        for (File file : all) {
            String dest = getBinary(file, dst);
            File destFile = new File(dest);
            if (destFile.isFile()
                    && (destFile.lastModified() > file.lastModified())) {
                continue;
            }
            result.add(file);
        }
        return result;
    }

    private void compile(File file, File dst) {
        String dest = getBinary(file, dst);
        File destFile = new File(dest);
        if (destFile.isFile()
                && (destFile.lastModified() > file.lastModified())) {
            return;
        }

        int relativePos = getProject().getBaseDir().getPath().length() + 1;
        System.out.println("\t" + file.getPath().substring(relativePos));
        List<String> args = new ArrayList<String>();
        args.add(sublagec);
        if ((debug != null) && (!debug.isEmpty())) {
            args.add("-d");
        }
        args.add(file.getAbsolutePath());
        args.add(dest);
        ProcessBuilder proc = new ProcessBuilder(args);
        proc.directory(getProject().getBaseDir());
        Map<String, String> env = proc.environment();
        env.put("LD_LIBRARY_PATH",
                dst.getPath() + File.pathSeparator + env.get("LD_LIBRARY_PATH"));
        if (!start(proc)) {
            throw new BuildException("Error while compiling `"
                    + file.getAbsolutePath() + "`:");
        }
    }

    public @Override
    void execute() throws BuildException {
        File dst = new File(getProject().getBaseDir() + File.separator + destdir);
        if (!dst.isDirectory()) {
            throw new BuildException(dst + " is not a directory or does not exists");
        }
        Collection<File> all = new ArrayList<File>();
        addTree(new File(getProject().getBaseDir() + File.separator + srcdir), all);
        List<File> modified = onlyModified(sortedDependencies(all), dst);
        if (!modified.isEmpty()) {
            String pluriel = (modified.size() == 1 ? "" : "s");
            System.out.println("Compiling " + modified.size() + " source" + pluriel
                    + " file" + pluriel + " to " + dst);
            for (File source : modified) {
                compile(source, dst);
            }
        }
    }
    
}
