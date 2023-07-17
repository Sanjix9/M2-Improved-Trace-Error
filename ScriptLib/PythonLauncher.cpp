void CPythonLauncher::SetTraceFunc(int (*pFunc)(PyObject * obj, PyFrameObject * f, int what, PyObject *arg))
{
	PyEval_SetTrace(pFunc, NULL);
}

//@Search, add under

static PyObject* print_binary(PyObject* self, PyObject* args)
{
	const char* msg;
	if (!PyArg_ParseTuple(args, "s", &msg))
		return NULL;

	PyThreadState *tstate = PyThreadState_GET();
	if (tstate == NULL || tstate->frame == NULL) {
		PyErr_SetString(PyExc_RuntimeError, "Failed to get current frame");
		return NULL;
	}

	PyFrameObject *frame = tstate->frame;
	int line = PyCode_Addr2Line(frame->f_code, frame->f_lasti);
	PyObject *filename_obj = frame->f_code->co_filename;

	PyObject *string_filename = PyObject_Str(filename_obj);
	if (string_filename == NULL) {
		PyErr_SetString(PyExc_TypeError, "Failed to convert PyObject to PyString");
		return NULL;
	}

	const char *filename = PyString_AsString(string_filename);
	if (filename == NULL) {
		PyErr_SetString(PyExc_TypeError, "Failed to get string from PyString object");
		Py_DECREF(string_filename);
		return NULL;
	}

	char buffer[1024];
	snprintf(buffer, sizeof(buffer), "[Name:%s Line:%d] %s", filename, line, msg);
	TraceError(buffer);

	Py_DECREF(string_filename);

	Py_RETURN_NONE;
}

static PyMethodDef PrintBinaryMethods[] = {
    {"printb", print_binary, METH_VARARGS, "*"},
    {NULL, NULL, 0, NULL}
};

///////////////////////////////////////////////////////////////
//////////----------------------------------///////////////////
///////////////////////////////////////////////////////////////

bool CPythonLauncher::Create(const char* c_szProgramName)
{
	Py_SetProgramName((char*)c_szProgramName);
#ifdef _DEBUG
	PyEval_SetTrace(TraceFunc, NULL);
#endif
	m_poModule = PyImport_AddModule((char *) "__main__");

	if (!m_poModule)
		return false;

	m_poDic = PyModule_GetDict(m_poModule);

    PyObject * builtins = PyImport_ImportModule("__builtin__");
	PyModule_AddIntConstant(builtins, "TRUE", 1);
	PyModule_AddIntConstant(builtins, "FALSE", 0);
    PyDict_SetItemString(m_poDic, "__builtins__", builtins);


//@Search, add under

	//@M2NewsCode010
	PyMethodDef* def;
	for (def = PrintBinaryMethods; def->ml_name != NULL; def++) {
		PyObject* func = PyCFunction_New(def, NULL);
		PyObject* builtins_dict = PyModule_GetDict(builtins);
		PyDict_SetItemString(builtins_dict, def->ml_name, func);
		Py_DECREF(func);
	}
	//@M2NewsCode010