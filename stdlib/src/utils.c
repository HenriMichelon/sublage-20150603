#include "utils.h"

void util_printStackObject(VmContext *vc, FILE *file, StackObject *so) {
    switch (so->opcode) {
        case OPCODE_NULL:
            fprintf(file, "null");
            break;
        case OPCODE_STRING:
            fprintf(file, "%s", vmContextGetString(vc, so));
            break;
        case OPCODE_INT:
            fprintf(file, "%lld", so->data.intValue);
            break;
        case OPCODE_FLOAT:
            fprintf(file, "%f", so->data.floatValue);
            break;
        case OPCODE_BOOLEAN:
            fprintf(file, "%s", (so->data.booleanValue ? "true" : "false"));
            break;
        case OPCODE_ARRAY:
        {
            fprintf(file, "[ ");
            LinkedList *array = vmContextGetArray(vc, so);
            if (array != NULL) {
                LinkedListIterator *it = linkedListCreateIterator(array);
                StackObject *elem = NULL;
                while ((elem = linkedListIteratorNext(it)) != NULL) {
                    util_printStackObject(vc, file, elem);
                    fprintf(file, " ");
                }
                linkedListIteratorDestroy(it);
            }
            fprintf(file, "]");
            break;
        }
        case OPCODE_INTERNALCALL_REF:
        {
            fprintf(file, "@InternalCall");
            InternalFunctionRef *ref = (InternalFunctionRef*)so->data.privateData;
            /*fprintf(file, "@InternalCall 0x%04x:0x%08llx",
                    ref->imageIndex,
                    ref->functionOffset);*/
            break;
        }
        case OPCODE_EXTERNALCALL_REF:
        {
            ExternalFunction *ef = vmContextGetCurrentImage(vc)->externals[so->data.importIndex];
            fprintf(file, "@0x%04x:0x%08llx", ef->imageIndex, ef->offset);
            break;
        }
        case OPCODE_NATIVECALL_REF:
        {
            NativeFunction *nf = vmContextGetCurrentImage(vc)->natives[so->data.nativeIndex];
            fprintf(file, "@0x%04x:0x%p", nf->imageIndex, (char*) nf->function);
            break;
        }
        case OPCODE_CLASSREF:
        {
            BinExecImg *img = vmContextGetCurrentImage(vc);
            ClassPointer *cp = linkedListGet(img->bef->classes, so->data.classIndex);
            fprintf(file, "@%s", cp->name);
            break;
        }
        case OPCODE_INSTANCE:
        {
            ClassInstance *object = so->data.privateData;
            fprintf(file, "%s[0x%04llx]", object->cp->name,
                     so->data.privateData);
            break;
        }
        default:
            fprintf(file, "???");
            break;

    }
}
