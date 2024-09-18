package sublage.debugger;

import sublage.debugger.objects.SublageVariable;
import sublage.debugger.objects.SublageBreakpoint;
import sublage.debugger.objects.SublageThread;
import sublage.debugger.views.VariablesModel;
import sublage.debugger.views.ThreadsModel;
import sublage.debugger.annotations.CurrentLineAnnotation;
import sublage.project.SublageProject;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;
import org.netbeans.api.debugger.Breakpoint;
import org.netbeans.api.debugger.DebuggerManager;
import org.netbeans.spi.viewmodel.TreeModel;
import org.openide.awt.StatusDisplayer;

public class Debugger implements Runnable {

    private interface ResponseListener {

        public void handleResponse(List<String> response);
    }

    private class Command {

        private String command;
        private ResponseListener listener;
        private List<String> response;

        public Command(String command, ResponseListener listener) {
            this.command = command;
            this.listener = listener;
        }

        public boolean send(BufferedReader in, PrintStream out) throws IOException {
            if (command.isEmpty()) {
                return false;
            }
            out.println(command);
            if (listener != null) {
                if (!recvList(in)) {
                    return false;
                }
                listener.handleResponse(response);
            } else {
                in.readLine(); // ACK
            }
            return true;
        }

        private boolean recvList(BufferedReader in) throws IOException {
            response = new LinkedList<String>();
            String line;
            do {
                line = in.readLine();
                if (line == null) {
                    return false;
                }
                if (!line.isEmpty()) {
                    response.add(line);
                }
            } while (debugging && (!line.isEmpty()));
            return true;
        }
    }
    public static final String THREADS_VIEW_NAME = "ThreadsView";
    public static final String VARIABLES_VIEW_NAME = "LocalsView";
    private ServerSocket serverSocket;
    private Socket socket;
    private boolean debugging;
    private IDEBridge ide;
    private BufferedReader in;
    private PrintStream out;
    private int currentThreadNumber = 1;
    private SublageThread currentThread = null;
    private VariablesModel varsModel;
    private ThreadsModel threadsModel;
    private final BlockingQueue<Command> queue = new LinkedBlockingQueue<Command>();
    private Command cmdThreadPaused = new Command("thread list", new ResponseListener() {
        List<SublageThread> threads = new LinkedList<SublageThread>();

        @Override
        public void handleResponse(List<String> response) {
            threads.clear();
            boolean inpause = false;
            for (String state : response) {
                SublageThread thread = new SublageThread(state);
                if (thread.getId() == currentThreadNumber) {
                    if (!thread.isRunning()) {
                        inpause = true;
                        if ((currentThread != null)
                                && (currentThread.getCodeIndex().equals(thread.getCodeIndex()))) {
                            threads.add(currentThread);
                            continue;
                        }
                        currentThread = thread;
                        ide.showAndAnnotateCurrentLine(thread.getSource(),
                                thread.getLine());
                        StatusDisplayer.getDefault().setStatusText("Current instruction: "
                                + thread.getText());
                    }
                } else if (!thread.isRunning()) {
                    // no multihread debugging support
                    sendCommand("thread continue " + thread.getId());
                    thread.setRunning(true);
                }
                threads.add(thread);
            }
            threadsModel.setThreads(threads);
            ide.setActionsInPause(inpause);
            if (!inpause) {
                ide.removeAnnotations(CurrentLineAnnotation.CURRENT_LINE_ANNOTATION_TYPE);
            } else {
                try {                
                    cmdVariables.send(in, out);
                } catch (IOException ex) {
                }
            }
        }
    });
    private Command cmdVariables = new Command("var list", new ResponseListener() {
        @Override
        public void handleResponse(List<String> response) {
            List<SublageVariable> vars = new ArrayList<SublageVariable>(response.size());
            for (String state : response) {
                vars.add(new SublageVariable(state));
            }
            varsModel.setVariabless(vars);
        }
    });

    private void sendCommand(String command) {
        queue.add(new Command(command, null));
    }

    public void commandStop() {
        ide.removeAnnotations();
        sendCommand("stop");
    }

    public void commandRun() {
        ide.removeAnnotations(CurrentLineAnnotation.CURRENT_LINE_ANNOTATION_TYPE);
        sendCommand("thread continue " + currentThreadNumber);
    }

    private void commandStep(String step) {
        sendCommand("thread step " + step + " " + currentThreadNumber);
    }

    public void commandStepOver() {
        commandStep("over");
    }

    public void commandStepInto() {
        commandStep("into");
    }

    public void commandStepOut() {
        commandStep("out");
    }

    public void commandPause() {
        sendCommand("thread pause " + currentThreadNumber);
    }

    public void commandBreakpointSet(SublageBreakpoint bp) {
        sendCommand("break set " + bp.getFilename() + " "
                + (bp.getLine().getLineNumber() + 1));
    }

    public void commandBreakpointUnset(SublageBreakpoint bp) {
        sendCommand("break unset " + bp.getFilename() + " "
                + (bp.getLine().getLineNumber() + 1));
    }

    public boolean isDebugging() {
        return debugging;
    }

    public void stop() {
        debugging = false;
        sendCommand("");
        ide.removeAnnotations();
        try {
            if (out != null) {
                out.close();
            }
            if (in != null) {
                in.close();
            }
            if (socket != null) {
                socket.close();
            }
            if (serverSocket != null) {
                serverSocket.close();
            }
        } catch (IOException ex) {
        }
    }

    public Debugger(SublageProject project, IDEBridge ide) throws IOException {
        this.ide = ide;
        serverSocket = new ServerSocket(0);
        serverSocket.setReuseAddress(true);
    }

    private void sendBreakpoints() throws IOException {
        for (Breakpoint breakpoint : DebuggerManager.getDebuggerManager().getBreakpoints()) {
            if (breakpoint instanceof SublageBreakpoint) {
                SublageBreakpoint bp = (SublageBreakpoint) breakpoint;
                out.println("break set " + bp.getFilename() + " "
                        + (bp.getLine().getLineNumber() + 1));
                in.readLine(); // ACK
            }
        }
    }

    @Override
    public void run() {
        try {
            ide.setActionsInPause(false);
            socket = serverSocket.accept();
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            out = new PrintStream(socket.getOutputStream());
            debugging = true;
            threadsModel = (ThreadsModel) DebuggerManager.getDebuggerManager().
                    getCurrentEngine().lookupFirst(THREADS_VIEW_NAME, TreeModel.class);
            varsModel = (VariablesModel) DebuggerManager.getDebuggerManager().
                    getCurrentEngine().lookupFirst(VARIABLES_VIEW_NAME, TreeModel.class);
            if ((threadsModel == null) || (varsModel == null)) {
                return;
            }
            threadsModel.setCurrentThread(currentThreadNumber);
            // welcome message
            StatusDisplayer.getDefault().setStatusText(in.readLine());
            sendBreakpoints();
            out.println("run");
            in.readLine(); // ACK
            while (debugging) {
                if (!cmdThreadPaused.send(in, out)) {
                    break;
                }
                Command cmd = queue.poll(250L, TimeUnit.MILLISECONDS);
                if (cmd != null) {
                    if (!cmd.send(in, out)) {
                        break;
                    }
                }
            }
        } catch (IOException ex) {
           //ex.printStackTrace();
        } catch (InterruptedException ex) {
        }
        stop();
    }

    public int getPort() {
        return serverSocket.getLocalPort();
    }
}
