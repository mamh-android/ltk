/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include <Python.h>
#include "STAFString.h"

extern "C"
{

#if defined(STAF_OS_NAME_HPUX) && !defined(__ia64)
    void _main();
#endif

struct module_state {
    PyObject *error;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

/****************************************/
/*  Python Wrappers for libSTAF functions  */
/****************************************/

static PyObject * PySTAFRegister(PyObject *self, PyObject *argv);
static PyObject * PySTAFSubmit(PyObject *self, PyObject *argv);
static PyObject * PySTAFUnregister(PyObject *self, PyObject *argv);
static PyObject * PySTAFAddPrivacyDelimiters(PyObject *self, PyObject *argv);
static PyObject * PySTAFEscapePrivacyDelimiters(PyObject *self, PyObject *argv);
static PyObject * PySTAFMaskPrivateData(PyObject *self, PyObject *argv);
static PyObject * PySTAFRemovePrivacyDelimiters(PyObject *self, PyObject *argv);

static PyMethodDef PYSTAF_Methods[] =
{
    { "STAFRegister", PySTAFRegister, METH_VARARGS, "Register with STAF" },
    { "STAFSubmit", PySTAFSubmit, METH_VARARGS, "Submit a STAF request" },
    { "STAFUnregister", PySTAFUnregister, METH_VARARGS, "Unregister with STAF" },
    { "STAFAddPrivacyDelimiters", PySTAFAddPrivacyDelimiters,
       METH_VARARGS, "Add Privacy Delimiters" },
    { "STAFEscapePrivacyDelimiters", PySTAFEscapePrivacyDelimiters,
       METH_VARARGS, "Escape Privacy Delimiters" },
    { "STAFMaskPrivateData", PySTAFMaskPrivateData,
       METH_VARARGS, "Mask Private Data" },
    { "STAFRemovePrivacyDelimiters", PySTAFRemovePrivacyDelimiters,
       METH_VARARGS, "Remove Privacy Delimiters" },
    { 0, 0, 0, 0 }
};

#if PY_MAJOR_VERSION >= 3

static int myextension_traverse(PyObject *m, visitproc visit, void *arg) {
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}

static int myextension_clear(PyObject *m) {
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}


static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "myextension",
        NULL,
        sizeof(struct module_state),
        PYSTAF_Methods,
        NULL,
        myextension_traverse,
        myextension_clear,
        NULL
};

#define INITERROR return NULL

PyObject *
PyInit_PYSTAF(void)
#else
#define INITERROR return
void initPYSTAF(void) 
#endif

/********************************************************************/
/* Initialization Procedure called when package is loaded in script */
/********************************************************************/

//void initPYSTAF(void) 
{
    #if defined(STAF_OS_NAME_HPUX) && !defined(__ia64)
        _main();
    #endif

    #if PY_MAJOR_VERSION >= 3
        PyObject *module = PyModule_Create(&moduledef);
    #else
        PyObject *module = Py_InitModule("PYSTAF", PYSTAF_Methods);
    #endif

    if (module == NULL)
        INITERROR;
    struct module_state *st = GETSTATE(module);

    st->error = PyErr_NewException("myextension.Error", NULL, NULL);
    if (st->error == NULL) {
        Py_DECREF(module);
        INITERROR;
    }

#if PY_MAJOR_VERSION >= 3
    return module;
#endif
}


/******************************************/
/* Implementation of Wrappers             */
/******************************************/

PyObject * PySTAFRegister(PyObject *self, PyObject *argv)
{
    char *name = 0;

    if (!PyArg_ParseTuple(argv, "s", &name)) return NULL;

    STAFHandle_t handle = 0;
    STAFRC_t rc = STAFRegister(name, &handle);

    return Py_BuildValue("(ii)", rc, handle);
}


PyObject * PySTAFSubmit(PyObject *self, PyObject *argv)
{
    static char empty_string[1] = { 0 };
    STAFHandle_t handle = 0;
    STAFSyncOption_t syncOption = kSTAFReqSync;
    char *machine = 0;
    char *service = 0;
    char *request = 0;
    unsigned int requestLength = 0;

    if (!PyArg_ParseTuple(argv, "iisss#", &handle, &syncOption, &machine,
                          &service, &request, &requestLength))
    {
        return NULL;
    }

    char *result = 0;
    unsigned int resultLength = 0;
    STAFRC_t rc;
    Py_BEGIN_ALLOW_THREADS;
    rc = STAFSubmit2(handle, syncOption, machine, service, request,
                     requestLength, &result, &resultLength);
    Py_END_ALLOW_THREADS;

    char *result2 = (result == 0) ? empty_string : result;
    PyObject *pyResult = Py_BuildValue("(is#)", rc, result2, resultLength);

    STAFFree(handle, result);

    return pyResult;
}


PyObject * PySTAFUnregister(PyObject *self, PyObject *argv)
{
    STAFHandle_t handle = 0;

    if (!PyArg_ParseTuple(argv, "i", &handle)) return NULL;

    STAFRC_t rc = STAFUnRegister(handle);

    return Py_BuildValue("i", rc);
}


static char sInfoBuffer[256] = { 0 };
static int sInfoBufferInited = 0;

char *STAFGetInformation()
{
   if (!sInfoBufferInited)
   {
       sprintf(sInfoBuffer, "%s STAF Python support library version 1.0",
               #ifdef NDEBUG
                   "Retail");
               #else
                   "Debug");
               #endif
       sInfoBufferInited = 1;
   }

   return sInfoBuffer;
}

PyObject * PySTAFAddPrivacyDelimiters(PyObject *self, PyObject *argv)
{
    char *data = 0;

    if (!PyArg_ParseTuple(argv, "s", &data)) return NULL;

    // Need to add a null character at the end
    STAFString result = STAFHandle::addPrivacyDelimiters(
        STAFString(data)) + STAFString(kUTF8_NULL);
    
    return Py_BuildValue("s", result.toCurrentCodePage()->buffer());
}

PyObject * PySTAFEscapePrivacyDelimiters(PyObject *self, PyObject *argv)
{
    char *data = 0;

    if (!PyArg_ParseTuple(argv, "s", &data)) return NULL;

    // Need to add a null character at the end
    STAFString result = STAFHandle::escapePrivacyDelimiters(
        STAFString(data)) + STAFString(kUTF8_NULL);
    
    return Py_BuildValue("s", result.toCurrentCodePage()->buffer());
}

PyObject * PySTAFMaskPrivateData(PyObject *self, PyObject *argv)
{
    char *data = 0;

    if (!PyArg_ParseTuple(argv, "s", &data)) return NULL;

    // Need to add a null character at the end
    STAFString result = STAFHandle::maskPrivateData(
        STAFString(data)) + STAFString(kUTF8_NULL);
    
    return Py_BuildValue("s", result.toCurrentCodePage()->buffer());
}

PyObject * PySTAFRemovePrivacyDelimiters(PyObject *self, PyObject *argv)
{
    char *data = 0;
    int numLevels = 0;

    if (!PyArg_ParseTuple(argv, "si", &data, &numLevels)) return NULL;

    // Need to add a null character at the end
    STAFString result = STAFHandle::removePrivacyDelimiters(
        STAFString(data), numLevels) + STAFString(kUTF8_NULL);
    
    return Py_BuildValue("s", result.toCurrentCodePage()->buffer());
}

} // end extern "C"
