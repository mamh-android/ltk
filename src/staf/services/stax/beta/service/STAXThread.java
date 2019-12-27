/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.HashMap;
import java.util.TreeMap;
import java.util.TreeSet;
import java.util.List;
import java.util.LinkedList;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Map;
import java.util.Set;
import java.util.LinkedHashSet;
import java.util.LinkedHashMap;
import java.util.Comparator;
import java.io.PrintStream;
import org.python.util.PythonInterpreter;
import org.python.core.*;
import com.ibm.staf.STAFMarshallingContext;

public class STAXThread implements STAXThreadCompleteListener
{
    // Thread states
    public static final int STATE_RUNNABLE = 0;
    public static final int STATE_RUNNING = 1;
    public static final int STATE_BLOCKED = 2;
    public static final int STATE_COMPLETE = 3;
    public static final int STATE_INIT = 4;

    public static final String STATE_INIT_STRING = new String("Init");
    public static final String STATE_RUNNABLE_STRING = new String("Runnable");
    public static final String STATE_RUNNING_STRING = new String("Running");
    public static final String STATE_BLOCKED_STRING = new String("Blocked");
    public static final String STATE_COMPLETE_STRING = new String("Complete");
    public static final String STATE_UNKNOWN_STRING = new String("Unknown");

    // Completion codes
    public static final int THREAD_END_OK = 0;
    public static final int THREAD_END_DUPLICATE_SIGNAL = 1;
    public static final int THREAD_END_STOPPED_BY_PARENT = 2;

    // How many non-blocking actions may a thread execute before blocking
    public static final int MAX_NON_BLOCKING_ACTIONS = 100;

    // Turn on/off thread debugging
    private static final boolean STAX_DEBUG_THREAD = false;

    public static final String THREAD = new String("thread");
    
    // For each thread, import Java and Jython modules and define Jython
    // classes and functions such as the STAXGlobal class and the CloneGlobals
    // function
    
    /* Jython 2.1:
    private static final String THREAD_INIT_PYCODE =
      "from com.ibm.staf import STAFResult as STAFRC \n" +
      "import STAFMarshalling \n" +
      "from STAFMarshalling import STAFMapClassDefinition, STAFMarshallingContext \n" +
      "import copy, types \n" +
      "import org.python.core.PyFunction \n" +

      // Save the Python built-in function named type in a variable
      // named STAXBuiltinFunction_type in case a STAX xml file sets
      // the type variable to something else.  Instead of type, use
      // STAXBuiltinFunction_type in the STAXCloneGlobals function.

      "STAXBuiltinFunction_type = type \n" +

      // Import the copy, types, and PyFunction modules in the CloneGlobals
      // function so that if someone uses a variable named copy or types
      // within a STAX job it won't interfere with these module names.

      "def STAXPythonFunction_CloneGlobals(STAXGlobals): \n" +
      "  import copy, types \n" +
      "  import org.python.core.PyFunction \n" +
      "\n" +
      "  STAXclone = copy.copy(STAXGlobals) \n" +
      "  STAXskipTypes = [types.ModuleType, types.FunctionType, \n" +
      "                   types.ClassType, \n" +
      "                   # since no StringMapType, use type(STAXclone) \n" +
      "                   STAXBuiltinFunction_type(STAXclone)] \n" +
      "  for STAXkey in STAXclone.keys(): \n" +
      "    if (STAXBuiltinFunction_type(STAXclone[STAXkey]) not in STAXskipTypes): \n" +
      "      try: \n" +
      "        STAXclone[STAXkey] = copy.deepcopy(STAXclone[STAXkey])\n" +
      "      except: \n" +
      "        pass  # ignore types that cannot be deepcopied \n" +

      // This section checks to see if the variable is a function definition
      // that was defined at the "global" scope, i.e., not within some other
      // Python module.  If so, then it replaces the function definition
      // with one that is identical except for the reference to the global
      // variable pool, which is redirected to look at the new global
      // variable pool being created.
      //
      // This was necessary, otherwise any variables created in the clone
      // are not accessable to functions defined before the clone.

      // Note, the "func_defaults or []" is necessary due to a bug in
      // Jython 2.1.  The PyFunction constructor doesn't handle a Null
      // parameter for func_defaults.  Note that it does handle func_closure
      // appropriately.

      "    elif ((STAXBuiltinFunction_type(STAXclone[STAXkey]) is types.FunctionType) and \n" +
      "          (STAXclone[STAXkey].func_globals is STAXGlobals)): \n" +
      "      STAXclone[STAXkey] = org.python.core.PyFunction(\n" +
      "          STAXclone,\n" +
      "          STAXclone[STAXkey].func_defaults or [],\n" +
      "          STAXclone[STAXkey].func_code,\n" +
      "          STAXclone[STAXkey].func_doc,\n" +
      "          STAXclone[STAXkey].func_closure)\n" +
      "  return STAXclone \n" +
    */
    // Jython 2.5:
    private static final String THREAD_INIT_PYCODE =
      "from com.ibm.staf import STAFResult as STAFRC\n" +
      "import STAFMarshalling\n" +
      "from STAFMarshalling import STAFMapClassDefinition, STAFMarshallingContext\n" +
      "import copy, types\n" +
      "from java.lang import Object\n" +
      // Save the Python built-in function "type" and imported copy and types
      // modules in variables beginning with STAX in case a STAX xml file sets
      // a variable named type, copy, or types to something else.  Use these
      // STAXxxx names instead in the STAXPythonFunction_CloneGlobals function.
      "STAXBuiltinFunction_type = type\n" +
      "STAXCopyModule = copy\n" +
      "STAXTypesModule = types\n" +
      "\n" +
      "def STAXPythonFunction_CloneGlobals(STAXGlobals):\n" +
      "  STAXclone = STAXCopyModule.copy(STAXGlobals)\n" +
      // Since no StringMapType, use type(STAXclone) to get its type to skip
      "  STAXStringMapType = STAXBuiltinFunction_type(STAXclone)\n" +
      "  for STAXkey in STAXclone.keys():\n" +
      "    STAXkeyType = STAXBuiltinFunction_type(STAXclone[STAXkey])\n" +
      "    #if isinstance(STAXclone[STAXkey], Object):\n" +
      "    #  print '%s with type %s is a Java object' % (STAXkey, STAXkeyType)\n" +
      "    #else:\n" +
      "    #  print '%s with type %s is not a Java object' % (STAXkey, STAXkeyType)\n" +
      "    if (STAXkeyType not in [STAXTypesModule.ModuleType,\n" +
      "                            STAXTypesModule.FunctionType,\n" +
      "                            STAXTypesModule.ClassType,\n" +
      "                            STAXStringMapType]):\n" +
      "      try:\n" +
      "        STAXclone[STAXkey] = STAXCopyModule.deepcopy(STAXclone[STAXkey])\n" +
      "      except:\n" +
      "        pass  # ignore types that cannot be deepcopied\n" +
      "    elif ((STAXkeyType is STAXTypesModule.FunctionType) and\n" +
      "          (STAXclone[STAXkey].func_globals is STAXGlobals)):\n" +
      //     This section checks to see if the variable is a function that was
      //     defined at the "global" scope, i.e., not within some other Python
      //     module.  If so, then it replaces the function definition with one
      //     that is identical except for the reference to the global variable
      //     pool, which is redirected to look at the new global variable pool
      //     being created.  This was necessary, otherwise any variables
      //     created in the clone are not accessable to functions defined
      //     before the clone.
      "      STAXclone[STAXkey] = STAXTypesModule.FunctionType(\n" +
      "          STAXclone[STAXkey].func_code,\n" +
      "          STAXclone,  # globals\n" +
      "          STAXclone[STAXkey].func_name,\n" +
      "          STAXclone[STAXkey].func_defaults,\n" +
      "          STAXclone[STAXkey].func_closure)\n" +
      "  return STAXclone\n" +
      "\n" +
      "def STAXPythonFunction_FunctionExists(STAXfunction): \n" +
      "  return STAXJob.functionExists(STAXfunction) \n" +
      "\n" +
      "class STAXUnique: \n" +
      "  def __init__(self, name): \n" +
      "    self.name = name \n" +
      "  def __str__(self): \n" +
      "    return str(self.name) \n" +
      "  def __repr__(self): \n" +
      "    return 'STAXUnique(%s)' % self.name \n" +
      "\n" +
      "STAXFunctionError = STAXUnique('STAXFunctionError') \n" +
      "\n" +
      "STAXNoResponseFromMachine = " +
      "STAXUnique('STAXNoResponseFromMachine') \n" +
      "\n" +
      "STAXFileCopyError = STAXUnique('STAXFileCopyError') \n" +
      "\n" +
      "STAXXMLParseError = STAXUnique('STAXXMLParseError') \n" +
      "\n" +
      "STAXImportModeError = STAXUnique('STAXImportModeError') \n" +
      "\n" +
      "STAXImportDirectoryError = STAXUnique('STAXImportDirectoryError') \n" +
      "\n" +
      "class STAXExceptionSource: \n" +
      "  def __init__(self, value = None): \n" +
      "     self.data = value \n" +
      "  def __str__(self): \n" +
      "     return 'Source: %s\\n\\n===== Stack Trace =====\\n\\n%s' % (self.data['source'], STAFMarshalling.formatObject(self.data['stackTrace'])) \n" +
      "  def __repr__(self): \n" +
      "    return repr(self.data) \n" +
      "  def __getitem__(self, key): \n" +
      "    return self.data[key] \n" +
      "  def getSource(self): \n" +
      "    return self.data['source'] \n" +
      "  def getStackTrace(self): \n" +
      "    return self.data['stackTrace'] \n" +
      "\n" +
      "class STAXGlobal: \n" +
      "\n\n" +
      "  # Basic customization \n" +
      "\n\n" +
      "  # constructor, optional value \n" +
      "  def __init__(self, value = None): \n" +
      "    self.data = value \n" +
      "\n" +
      "  def __del__(self): \n" +
      "    del self \n" +
      "\n" +
      "  # returns a string representation \n" +
      "  def __repr__(self): \n" +
      "    return repr(self.data) \n" +
      "\n" +
      "  def __str__(self): \n" +
      "    return str(self.data) \n" +
      "\n" +
      "  def __lt__(self, other): \n" +
      "    if self.data < other: \n" +
      "      return 1 \n" +
      "    else: \n" +
      "      return 0 \n" +
      "\n" +
      "  def __le__(self, other): \n" +
      "    if self.data <= other: \n" +
      "      return 1 \n" +
      "    else: \n" +
      "      return 0 \n" +
      "\n" +
      "  def __eq__(self, other): \n" +
      "    if self.data == other: \n" +
      "      return 1 \n" +
      "    else: \n" +
      "      return 0 \n" +
      "\n" +
      "  def __ne__(self, other): \n" +
      "    if self.data != other: \n" +
      "      return 1 \n" +
      "    else: \n" +
      "      return 0 \n" +
      "\n" +
      "  def __gt__(self, other): \n" +
      "    if self.data > other: \n" +
      "      return 1 \n" +
      "    else: \n" +
      "      return 0 \n" +
      "\n" +
      "  def __ge__(self, other): \n" +
      "    if self.data >= other: \n" +
      "      return 1 \n" +
      "    else: \n" +
      "      return 0 \n" +
      "\n" +
      "  def __cmp__(self, other): \n" +
      "    return self.data.cmp(other) \n" +
      "\n" +
      "  def __hash__(self): \n" +
      "    try: \n" +
      "      return hash(self.data) \n" +
      "    except: \n" +
      "      # Unhashable type (e.g. list or dictionary) \n" +
      "      return hash(id(self.data)) \n" +
      "\n" +
      "  def __nonzero__(self): \n" +
      "    if self.data: \n" +
      "      return 1 \n" +
      "    else: \n" +
      "      return 0 \n" +
      "\n" +
      "  # Customizing attribute access \n" +
      "\n" +
      "  def __getattr__(self, name): \n" +
      "    return getattr(self.data, name) \n" +
      "\n" +
      /* Not implementing __setattr__ and __delattr__ yet
      "  # Not sure if these are the correct implementations \n" +
      "  def __setattr__(self, name, value): \n" +
      "    self.__dict__[name] = value \n" +
      "\n" +
      "  def __delattr__(self, name): \n" +
      "    del self.__dict__[name] \n" +
      "\n" +
      */
      "  # Emulating callable objects \n" +
      "\n" +
      "  # Not implementing __call__ \n" +
      "\n" +
      "  # Emulating container types \n" +
      "\n" + 
      "  def __len__(self): \n" +
      "    return len(self.data) \n" +
      "\n" +
      "  def __getitem__(self, key): \n" +
      "    return self.data[key] \n" +
      "\n" + 
      "  def __setitem__(self, key, value): \n" +
      "    self.data[key] = value \n" +  
      "\n" +
      "  def __delitem__(self, key): \n" +
      "    del self.data[key] \n" +
      "\n" +
      // Jython 2.5: iter() is new in Python 2.2 so uncomment if using 2.5+
      "  def __iter__(self): \n" +
      "    return iter(self.data) \n" +
      "\n" +
      "  def __contains__(self, item): \n" +
      "    return item in self.data \n" +
      "\n" +
      "  # Additional methods for emulation of sequence types \n" +
      "  # Note: Once the getattr method was added, also had to add \n" +
      "  # the slice methods (even though they are deprecated) \n" +
      "\n" +
      "  def __getslice__(self, i, j): \n" +
      "    return self.data[max(0, i):max(0, j):]" +
      "\n" +
      "  def __setslice__(self, i, j, seq): \n" +
      "    self.data[max(0, i):max(0, j):] = seq \n" +
      "\n" +
      "  def __delslice__(self, i, j): \n" +
      "    del self.data[max(0, i):max(0, j):]" +
      "\n" +
      "  # Emulating numeric types \n" +
      "  def __add__(self, other): \n" +
      "    return self.data + other \n" +
      "\n" +
      "  def __sub__(self, other): \n" +
      "    return self.data - other \n" +
      "\n" +
      "  def __mul__(self, other): \n" +
      "    return self.data * other \n" +
      "\n" +
      "  # Commented out - get SyntaxError when run \n" +
      "  # def __floordiv__(self, other): \n" +
      "  #   return self.data // other \n" +
      "\n" +
      "  def __mod__(self, other): \n" +
      "    return self.data % other \n" +
      "\n" +
      "  def __divmod__(self, other): \n" +
      "    return self.data.divmod(other) \n" +
      "\n" +
      "  def __pow__(self, other, modulo=None): \n" +
      "    return self.data.pow(other)      # ??? modulo \n" +
      "\n" +
      "  def __lshift__(self, other): \n" +
      "    return self.data << other \n" +
      "\n" +
      "  def __rshift__(self, other): \n" +
      "    return self.data >> other \n" +
      "\n" +
      "  def __and__(self, other): \n" +
      "    return self.data & other \n" +
      "\n" +
      "  def __xor__(self, other): \n" +
      "    return self.data ^ other \n" +
      "\n" +
      "  def __or__(self, other): \n" +
      "    return self.data | other \n" +
      "\n" +
      "  def __div__(self, other): \n" +
      "    return self.data / other \n" +
      "\n" +
      "  def __truediv__(self, other): \n" +
      "    return self.data / other \n" +
      "\n" + 
      "  def __radd__(self, other): \n" +
      "    return other + self.data \n" +
      "\n" + 
      "  def __rsub__(self, other): \n" +
      "    return other - self.data \n" +
      "\n" + 
      "  def __rmul__(self, other): \n" +
      "    return other * self.data \n" +
      "\n" + 
      "  def __rdiv__(self, other): \n" +
      "    return other / self.data \n" +
      "\n" + 
      "  def __rmod__(self, other): \n" +
      "    return other % self.data \n" +
      "\n" + 
      "  def __rdivmod__(self, other): \n" +
      "    return other.divmod(self.data) \n" +
      "\n" + 
      "  def __rpow__(self, other): \n" +
      "    return other.pow(self.data) \n" +
      "\n" + 
      "  def __rlshift__(self, other): \n" +
      "    return other << self.data \n" +
      "\n" + 
      "  def __rrshift__(self, other): \n" +
      "    return other >> self.data \n" +
      "\n" + 
      "  def __rand__(self, other): \n" +
      "    return other | self.data \n" +
      "\n" + 
      "  def __rxor__(self, other): \n" +
      "    return other ^ self.data \n" +
      "\n" + 
      "  def __ror__(self, other): \n" +
      "    return other | self.data \n" +
      "\n" + 
      "  def __iadd__(self, other): \n" +
      "    self.data += other \n" +
      "    return self \n" +
      "\n" + 
      "  def __isub__(self, other): \n" +
      "    self.data -= other \n" +
      "    return self \n" +
      "\n" + 
      "  def __imul__(self, other): \n" +
      "    self.data *= other \n" +
      "    return self \n" +
      "\n" + 
      "  def __idiv__(self, other): \n" +
      "    self.data /= other \n" +
      "    return self \n" +
      "\n" + 
      "  def __imod__(self, other): \n" +
      "    self.data %= other \n" +
      "    return self \n" +
      "\n" + 
      "  def __ipow__(self, other): \n" +
      "    self.data **= other \n" +
      "    return self \n" +
      "\n" + 
      "  def __ilshift__(self, other): \n" +
      "    self.data <<= other \n" +
      "    return self \n" +
      "\n" + 
      "  def __irshift__(self, other): \n" +
      "    self.data >>= other \n" +
      "    return self \n" +
      "\n" + 
      "  def __iand__(self, other): \n" +
      "    self.data &= other \n" +
      "    return self \n" +
      "\n" + 
      "  def __ixor__(self, other): \n" +
      "    self.data ^= other \n" +
      "    return self \n" +
      "\n" + 
      "  def __ior__(self, other): \n" +
      "    self.data |= other \n" +
      "    return self \n" +
      "\n" + 
      "  def __neg__(self): \n" +
      "    self.data = -self.data \n" +
      "    return self \n" +
      "\n" + 
      "  def __pos__(self): \n" +
      "    self.data = +self.data \n" +
      "    return self \n" +
      "\n" + 
      "  def __abs__(self): \n" +
      "    self.data = abs(self.data) \n" +
      "    return self \n" +
      "\n" + 
      "  def __invert__(self): \n" +
      "    self.data = ~self.data \n" +
      "    return self \n" +
      "\n" + 
      "  def __complex__(self): \n" +
      "    return complex(self.data) \n" +
      "\n" + 
      "  def __int__(self): \n" +
      "    return int(self.data) \n" +
      "\n" + 
      "  def __long__(self): \n" +
      "    return long(self.data) \n" +
      "\n" + 
      "  def __float__(self): \n" +
      "    return float(self.data) \n" +
      "\n" + 
      "  def __oct__(self): \n" +
      "    return oct(self.data) \n" +
      "\n" + 
      "  def __hex__(self): \n" +
      "    return hex(self.data) \n" +
      "\n" + 
      "  # XXX: Currently not defining __coerce__(self, other) \n" +
      "\n" +
      "  # Providing set and get methods \n" +
      "\n" +
      "  def set(self, value): \n" +    
      "    self.data = value \n" +
      "\n" +
      "  def get(self): \n" +
      "    return self.data \n" +
      "\n" +
      "  # Implementing deepcopy so will be global \n" +
      "  def __deepcopy__(self): \n" +
      "    return self \n" +
      "\n" +
      "  # Providing an append method \n" +
      "  def append(self, other): \n" +
      "    self.data.append(other) \n" +
      "    return self \n" +
      "\n" +
      "  # Provide a stafMarshall method \n" +
      "  def stafMarshall(self, context): \n" +
      "    import STAFMarshalling \n" +
      "    return STAFMarshalling.marshall(self.data, context) \n" +
      "";

    private STAXThread() { /* Do Nothing */ }

    // This creates another first-level thread for the job

    public STAXThread(STAXJob job)
    {
        fJob = job;
        fThreadNumber = job.getNextThreadNumber();

        // Create a PythonInterpreter for the main thread specifying null for
        // the local variables and specifying a new PySystemState.
        // Child threads that create a new PythonInterpreter will share the
        // same PySystemState.

        PySystemState pySystemState = new PySystemState();
        fPythonInterpreter = new STAXPythonInterpreter(null, pySystemState);

        fPythonInterpreter.set("STAXThreadID", new Integer(fThreadNumber));
        fPythonInterpreter.exec("STAXTestcaseStack = []");
        fPythonInterpreter.set("STAXCurrentTestcase", Py.None);
        fPythonInterpreter.exec("STAXBlockStack = []");
        fPythonInterpreter.set("STAXCurrentBlock", Py.None);

        // Run Jython code that imports Java and Jython modules and defines
        // Jython classes and functions such as the STAXGlobal class and the
        // CloneGlobals function

        fPythonInterpreter.exec(THREAD_INIT_PYCODE);

        // Get the current date and time and set as the starting date/time
        fStartTimestamp = new STAXTimestamp();
    }

    //
    // Thread functions
    //

    /**
     * Creates a child thread of this thread with a new Python Interpreter.
     * 
     * This method is the same as the createChildThread2() method except it
     * does not throw an exception if it exceeds maximum number of threads
     * allowed.
     */ 
    public STAXThread createChildThread()
    {
        return createChildThread(false);
    }

    /**
     * Creates a child thread of this thread with a new Python Interpreter.
     * 
     * This method is the same as the createChildThread() method except that
     * it throws an exception if it exceeds the maximum number of threads
     * allowed.
     * 
     * Note: This is called by actions like STAXParallelInterateAction and
     * STAXParallelAction so that if they try to run too many actions in
     * parallel, it will throw an exception if exceeds the maximum number of
     * threads allowed.
     */ 
    public STAXThread createChildThread2() throws STAXExceedsMaxThreadsException
    {
        return createChildThread2(false);
    }

    /**
     * Creates a child thread of this thread, adding the thread to the job's
     * thread map and to the parent thread's child thread set.
     * 
     * This method is the same as the createChildThread2 method except it does
     * not throw an exception if it exceeds maximum number of threads allowed.
     */ 
    public STAXThread createChildThread(boolean useSamePyInterpreter)
    {
        STAXThread child = initChildThread(useSamePyInterpreter);

        fJob.addThread(child);

        child.addCompletionNotifiee(this);
        
        synchronized (fChildThreadSet)
        {
            fChildThreadSet.add(child.getThreadNumberAsInteger());
        }
        
        return child;
    }
    
    /**
     * Creates a child thread of this thread, adding the thread to the job's
     * thread map and to the parent thread's child thread set.
     * 
     * This method is the same as the createChildThread method except that it
     * throws an exception if it exceeds the maximum number of threads allowed.
     */ 
    public STAXThread createChildThread2(boolean useSamePyInterpreter)
        throws STAXExceedsMaxThreadsException
    {
        STAXThread child = initChildThread(useSamePyInterpreter);

        fJob.addThreadIfDoesNotExceedMax(child);

        child.addCompletionNotifiee(this);
        
        synchronized (fChildThreadSet)
        {
            fChildThreadSet.add(child.getThreadNumberAsInteger());
        }
        
        return child;
    }
    
    /**
     * Initializes a child thread of this thread.
     * Called by the createChildThread and createChildThread2 methods.
     */
    private STAXThread initChildThread(boolean useSamePyInterpreter)
    {
        STAXThread child = new STAXThread();

        child.fJob = fJob;
        child.fThreadNumber = fJob.getNextThreadNumber();
        child.fParent = this;
        child.fSignalHandlerMap = (HashMap)fSignalHandlerMap.clone();
        child.fBreakpointCondition = fBreakpointCondition;

        synchronized (fPythonInterpreter)
        {
            if (!useSamePyInterpreter)
                child.fPythonInterpreter = fPythonInterpreter.clonePyi();
            else
                child.fPythonInterpreter = fPythonInterpreter;
        }

        child.fPythonInterpreter.set("STAXThreadID", 
                                     new Integer(child.fThreadNumber));

        // Assign the parent hierarchy string for the child thread.
        // For example, if this child's parent thread id is 5 and its
        // parent's parent thread hierarchy is 1.2, then it will be:
        //   1.2.5

        String hierarchy = child.fParent.getParentHierarchy();
        int parentThreadID = child.fParent.getThreadNumber();

        if (hierarchy.length() == 0)
            child.fThreadParentHierarchy = String.valueOf(parentThreadID);
        else
            child.fThreadParentHierarchy = hierarchy + "." + parentThreadID;

        // Get the current date and time and set as the starting date/time
        child.fStartTimestamp = new STAXTimestamp();

        return child;
    }

    // Note: When using the visitChild function, do not try to modify the
    //       iterator passed to you, as it will be null

    public void visitChildren(STAXVisitor visitor)
    {
        synchronized (fChildThreadSet)
        {
            Iterator iter = fChildThreadSet.iterator();

            while (iter.hasNext())
            {
                Integer threadNumber = (Integer)iter.next();
                visitor.visit(fJob.getThread(threadNumber), null);
            }
        }
    }

    public STAXThread getParentThread() { return fParent; }

    public STAXPythonInterpreter getPythonInterpreter()
    {
        synchronized(fPythonInterpreter)
        {
            return fPythonInterpreter;
        }
    }

    public void setPythonInterpreter(STAXPythonInterpreter pyi)
    {
        synchronized(fPythonInterpreter)
        {
            fPythonInterpreter = pyi;
        }
    }
    
    /**
     * Set the PythonInterpreter's stdout to a custom output stream.
     * This allows the custom output stream to redirect the output 
     * to the STAX Job Log and/or send a message to the STAX Monitor,
     * or reformat the output (prepend timestamp and job ID).
     * @param out A STAXPythonOutput object representing the custom output
     * stream for the STAX job
     */
    public void setPythonInterpreterStdout(STAXPythonOutput out)
    {
        // Must set the autoFlush boolean parameter to true so that the
        // output from each Jython print statement will appear as a separate
        // entry in a message logged in the STAX Job Log and/or in a message
        // sent to the STAX Monitor.
        fPythonInterpreter.setOut(new PrintStream(out, true));
    }

    /**
     * Set the PythonInterpreter's stderr to a custom output stream.
     * This allows the custom output stream to redirect the output 
     * to the STAX Job Log and/or send a message to the STAX Monitor,
     * or reformat the output (prepend timestamp and job ID).
     * @param out A STAXPythonOutput object representing the custom output
     * stream for the STAX job
     */
    public void setPythonInterpreterStderr(STAXPythonOutput out)
    {
        // Must set the autoFlush boolean parameter to true so that the
        // output from each Jython print statement will appear as a separate
        // entry in a message logged in the STAX Job Log and/or in a message
        // sent to the STAX Monitor.
        fPythonInterpreter.setErr(new PrintStream(out, true));
    }

    //
    // Miscellaneous information functions
    //

    public STAXJob getJob() { return fJob; }

    public int getThreadNumber() { return fThreadNumber; }

    public Integer getThreadNumberAsInteger()
    { return new Integer(fThreadNumber); }

    public STAXTimestamp getStartTimestamp() { return fStartTimestamp; }
    
    //
    // Completion notification function
    //

    public void addCompletionNotifiee(STAXThreadCompleteListener listener)
    {
        fCompletionNotifiees.addLast(listener);
    }

    //
    // Python Interpreter functions
    //

    // Set a Jython variable (e.g. RC, STAFResult) to a specified object.
 
    public void pySetVar(String varName, Object value)
    {
        synchronized(fPythonInterpreter)
        {
            try
            {
                fPythonInterpreter.set(varName, value);
            }
            catch (PyException e)
            {
                System.out.println("PySetVar PyException caught setting var=" +
                    varName + " to value=" + value + e.toString());
            } 
        }
    }

    // Execute Python code.
 
    public void pyExec(String value) throws STAXPythonEvaluationException
    {
        try
        {
            if (STAX.CACHE_PYTHON_CODE)
            {
                PyCode codeObject = fJob.getCompiledPyCode(value, "exec");

                synchronized(fPythonInterpreter)
                {
                    fPythonInterpreter.exec(codeObject);
                }
            }
            else
            {
                synchronized(fPythonInterpreter)
                {
                    fPythonInterpreter.exec(value);
                }
            }
        }
        catch (PyException e)
        {
            throw new STAXPythonEvaluationException(e.toString());
        } 
    }
    
    // Compile Python code (or get the compiled Jython code for the specified
    // code string from the cache if already compiled).
    // Note that we cache compiled Jython code so that if it is used more
    // than once, it eliminates the overhead of recompiling the code string
    // each time it is executed.

    public PyObject pyCompile(String codeString)
    {
        if (STAX.CACHE_PYTHON_CODE)
        {
            PyCode codeObject = fJob.getCompiledPyCode(codeString, "eval");

            synchronized(fPythonInterpreter)
            {
                /* Jython 2.1:
                fPythonInterpreter.exec(codeObject);
                return fPythonInterpreter.get("STAXPyEvalResult");
                */
                // Jython 2.5:
                return fPythonInterpreter.eval(codeObject);
            }
        }
        else
        {
            synchronized(fPythonInterpreter)
            {
                return fPythonInterpreter.eval(codeString);
            }
        }
    }

    // Evaluate a value using Jython and return a PyObject result.
     
    public PyObject pyObjectEval(String value) 
        throws STAXPythonEvaluationException
    {
        try 
        {
            return pyCompile(value);
        }
        catch (PyException e)
        {
            throw new STAXPythonEvaluationException(
                "\nPython object evaluation failed for:\n" +
                value + "\n\n" + e.toString());
        }
    }

    // Evaluate a value using Jython and return a String result.
     
    public String pyStringEval(String value) 
        throws STAXPythonEvaluationException
    {
        try 
        {
            return pyCompile(value).__str__().toString();
        }
        catch (PyException e)
        {
            throw new STAXPythonEvaluationException(
                "\nPython string evaluation failed for:\n" +
                value + "\n\n" + e.toString());
        }
    }

    // Evaluate an expression using Jython and return a boolean result.
 
    public boolean pyBoolEval(String expression)
        throws STAXPythonEvaluationException
    {
        try
        {  
            return Py.py2boolean(pyCompile(expression));
        }
        catch (PyException e)
        {
            throw new STAXPythonEvaluationException(
                "\nPython boolean evaluation failed for:\n" +
                expression + "\n\n" + e.toString());
        }
    }

    // Evaluate a value using Jython and return an integer result.
 
    public int pyIntEval(String value) throws STAXPythonEvaluationException
    {
        try
        {  
            return Py.py2int(pyCompile(value));
        }
        catch (PyException e)
        {
            throw new STAXPythonEvaluationException(
                "\nPython integer evaluation failed for:\n" +
                value + "\n\n" + e.toString());
        }
    }

    // Evaluate a value (which evaluates to a list or tuple) using Jython, 
    // extract the Jython list/tuple into a Java List, and return a List.
    // Examples of possible values passed in:
    //   "machList", ['machA','machB'], ('machA','machB',machC'),
    //   machList[1:], machList[:-1], ['machA','machB'] + ['machC','machD']  

    public List pyListEval(String value)
        throws STAXPythonEvaluationException
    {
        List jList = new ArrayList();

        synchronized(fPythonInterpreter)
        {
            PyObject result = null;

            try
            {
                result = pyCompile(value);
            }
            catch (PyException e)
            {
                throw new STAXPythonEvaluationException(
                    "\nPython list evaluation failed for:\n" +
                    value + "\n" + e.toString());
            }
            
            // Check if value is a STAXGlobal object

            try 
            {
                if (pyBoolEval("isinstance(" + value + ", STAXGlobal)"))
                {
                    // Use the STAXGlobal class's get() method to retrieve
                    // it's contents
                    result = pyCompile(value + ".get()");
                }
            }
            catch (PyException e)
            {
                // Ignore error and assume not a STAXGlobal object
            }

            // Check if already is a Java List object

            Object javaObj = result.__tojava__(Object.class);

            if (javaObj instanceof java.util.List)
                return (List)javaObj;
            
            try
            {
                // Convert Python object to a Java ArrayList object

                fPythonInterpreter.set("STAXIterateList", result);

                if (Py.py2boolean(result))
                {
                    jList =  new ArrayList(Arrays.asList(
                        (Object[])fPythonInterpreter.get(
                            "STAXIterateList", Object[].class)));
                }
            }
            catch (PyException e)
            {
                try
                {
                    jList = new ArrayList();
                    jList.add(javaObj);
                }
                catch (PyException e2)
                {
                    throw new STAXPythonEvaluationException(
                        "\nPython list evaluation failed for:\n" +
                        value + "\n" + e.toString() + "\n" + e2.toString());
                }
            } 
        }

        return jList;
    }

    // Evaluate a value (which evaluates to a Python dictionary) using Jython, 
    // extract the contents of the PyDictionary into a Java Map, and return a
    // Map.  
    // Example of possible value that can be passed in:
    //   "{'server': 'machineA', 'testDir': 'C:/tests'}"

    public Map pyMapEval(String value) throws STAXPythonEvaluationException
    {
        // Extract a Python dictionary into a Java Map

        // Create an empty HashMap
        Map jMap = new HashMap(); 

        try
        {
            PyObject result = pyCompile(value);

            // If a null string, an empty string, or "None" is passed in,
            // return an empty map.

            if ((result != null) && !(result.toString().equals("None")) &&
                (result.toString() != ""))
            {
                if (result instanceof PyDictionary)
                { 
                    PyList pa = ((PyDictionary)result).items();

                    while (pa.__len__() != 0)
                    {
                        PyTuple po = (PyTuple)pa.pop();
                        Object first  = 
                            po.__finditem__(0).__tojava__(Object.class);
                        PyObject second = po.__finditem__(1);
                        jMap.put(first, second);
                    }
                }
                else
                {
                    throw new STAXPythonEvaluationException(
                        "\nThe following string does not evaluate " +
                        "to a PyDictionary:\n" + value);
                }
            }
        }
        catch (PyException e)
        {
            throw new STAXPythonEvaluationException(
                "\nPython dictionary evaluation failed for:\n" + value +
                "\n" + e.toString());
        }
        
        return jMap;
    }

    /**
     * This method retrieves all local variables in the thread's Python
     * Interpreter and converts them into a Java Map whose keys are the
     * Python variable names and whose values are the variable values.
     */
    public Map getLocals()
    {
        Map jMap = new LinkedHashMap();

        synchronized(fPythonInterpreter)
        {
            try
            {
                PyObject result = fPythonInterpreter.getLocals();

                PyList pa = new PyList();

                /* Jython 2.1  getLocals() returns PyStringMap
                if (result instanceof PyStringMap)
                    pa = ((PyStringMap)result).copy().items();
                else if (result instanceof PyDictionary)
                    pa = ((PyDictionary)result).copy().items();
                */    
                // Jython 2.5.x  getLocals() returns PyDictionary not a
                //      PyStringMap as in Jython 2.1
                if (result instanceof PyDictionary)
                    pa = ((PyDictionary)result).copy().items();
                else if (result instanceof PyStringMap)
                    pa = ((PyStringMap)result).copy().items();
                else
                    System.out.println(
                        "STAXThread::getLocals() " +
                        "fPythonInterpreter.getLocals() returned a " +
                        "PyObject whose type couldn't be handled: " +
                        result);

                pa.sort();

                for (int i = 0; i < pa.__len__(); i++)
                {
                    PyTuple po = (PyTuple)pa.__getitem__(i);
                    Object first  = 
                        po.__finditem__(0).__tojava__(Object.class);
                    PyObject second = po.__finditem__(1);

                    try
                    {
                        if (pyBoolEval("isinstance(" + first + ", STAXGlobal)"))
                        {
                            // Use the STAXGlobal class's get() method to
                            // retrieve it's contents
                            second = pyCompile(first + ".get()");
                        }
                    }
                    catch (Exception e)
                    {
                        // Ignore error and assume not a STAXGlobal object
                    }

                    jMap.put(first, second);
                }
            }
            catch (PyException e)
            {
                System.out.println(
                    "STAXThread::getLocals() PyException caught\n" +
                    e.toString());
            }
        }

        return jMap;
    }

    //
    // Signal functions
    //

    public void registerSignalHandler(String name,
                                      STAXSignalExecutionAction signalHandler)
    {
        fSignalHandlerMap.put(name, signalHandler);
    }

    public void raiseSignal(String name)
    {
        raiseSignal(name, null);
    }

    public void raiseSignal(String name, STAXActionDefaultImpl raiserAction)
    {
        // Check for this signal already

        // XXX: Should also check if same signalhandler action
        if (fSignalStack.contains(name))
        {
            getJob().log(STAXJob.JOB_LOG, "error", "Duplicate signal " + name +
                         ".  Terminating job.");
            terminate(THREAD_END_DUPLICATE_SIGNAL);
            return;
        }

        STAXAction handlerAction = (STAXAction)fSignalHandlerMap.get(name);

        if (handlerAction == null)
        {
            if (name != "STAXNoSuchSignalHandler")
            {
                String msg = "No signal handler exists for signal: " + name;

                if (raiserAction != null)
                {
                    raiserAction.setElementInfo(new STAXElementInfo(
                        raiserAction.getElement(), "signal", msg));
                    
                    setSignalMsgVar(
                        "STAXNoSuchSignalHandlerMsg",
                        STAXUtil.formatErrorMessage(raiserAction));
                }
                else
                {
                    setSignalMsgVar("STAXNoSuchSignalHandlerMsg", msg);
                }

                raiseSignal("STAXNoSuchSignalHandler");
            } 
            else
            {   // Should not happen - needed to break recursion
                getJob().log(STAXJob.JOB_LOG, "error", "No signal " +
                    "handler exists for signal STAXNoSuchSignalHandler");
            }
        }
        else
        {
            pushAction(handlerAction.cloneAction());
            fSignalStack.addLast(name);
        }
    }

    /**
     * Generate a message containing information about why the signal
     * was raised using information provided.
     * 
     * @param signalMsgVarName The name of the signal message variable
     * 
     * @param xmlInfo Information about the element/[attribute] that
     * caused the signal to be raised
     */
    public void setSignalMsgVar(String signalMsgVarName, String xmlInfo)
    {
        setSignalMsgVar(signalMsgVarName, xmlInfo, null);
    }

    /**
     * Generate a message containing information about why the signal
     * was raised using information provided.
     * 
     * @param signalMsgVarName The name of the signal message variable
     * 
     * @param xmlInfo Information about the element/[attribute] that
     * caused the signal to be raised
     * 
     * @param pythonException The STAXPythonEvaluationException that was
     * raised
     */
    public void setSignalMsgVar(String signalMsgVarName,
                                String xmlInfo,
                                STAXPythonEvaluationException pythonException)
    {
        StringBuffer msg = new StringBuffer();

        if (xmlInfo != null)
        {
            msg.append("\n\n===== XML Information =====\n\n");
            msg.append(xmlInfo);
        }

        if (pythonException != null)
        {
            msg.append("\n\n===== Python Error Information =====\n\n");
            msg.append(pythonException);
            msg.append("\n");
        }
        else
        {
            msg.append("\n\n");
        }

        msg.append("===== Call Stack for STAX Thread ");
        msg.append(fThreadNumber);
        msg.append(" =====\n\n");
        msg.append(STAFMarshallingContext.formatObject(getCallStack()));

        List conditionStack = this.getConditionStack();

        if (conditionStack.size() > 0)
        {
            msg.append("\n\n==== Condition Stack for STAX Thread ");
            msg.append(fThreadNumber);
            msg.append(" =====\n\n");
            msg.append(STAFMarshallingContext.formatObject(
                getConditionStack()));
        }

        // Set the message value to the signal message variable name

        pySetVar(signalMsgVarName, msg.toString());
    }

    public void handledSignal(String name)
    {
        if ((fSignalStack.size() == 0) ||
            (!((String)fSignalStack.getLast()).equals(name)))
        {
            System.out.println("We aren't currently handling signal: " + name);
            // XXX: Not good
        }
        else fSignalStack.removeLast();
    }

    public List getCallStack()
    {
        // Generate the call stack

        List callStack = new ArrayList();

        this.visitActions(new STAXVisitorHelper(callStack)
        {
            public void visit(Object o, Iterator iter)
            {
                List callStack = (ArrayList)fData;

                if (o instanceof STAXActionDefaultImpl)
                {
                    STAXActionDefaultImpl action = (STAXActionDefaultImpl)o;

                    if ((o instanceof STAXBreakpointAction) &&
                        !(action.getInfo().equals("")))
                    {
                        // This a STAXBreakpointAction created for a
                        // Function or Line breakpoint, or due to a STEP
                        // request, so don't include this action in the
                        // call stack output
                        return;
                    }
                    callStack.add(STAXUtil.formatActionInfo(action));
                 }
                else
                {
                    STAXAction action = (STAXAction)o;

                    String className = STAXUtil.getShortClassName(
                        action.getClass().getName(), "Action");

                    callStack.add(className + ": " + action.getInfo());
                }
            }
        });

        return callStack;
    }

    public List getConditionStack()
    {
        // Generate the condition stack

        List conditionStack = new ArrayList();

        this.visitConditions(new STAXVisitorHelper(conditionStack)
        {
            public void visit(Object o, Iterator iter)
            {
                STAXCondition condition = (STAXCondition)o;

                List conditionStack = (ArrayList)fData;

                String className = STAXUtil.getShortClassName(
                    condition.getClass().getName(), "Condition");

                conditionStack.add(
                    className + ": Source=" + condition.getSource() +
                    ", Priority=" + condition.getPriority());
            }
        });

        return conditionStack;
    }

    public String getParentHierarchy()
    {
        return fThreadParentHierarchy;
    }

    //
    // State functions
    //

    public int getState()
    {
        return fState;
    }

    public String getStateAsString()
    {
        switch (fState)
        {
            case STATE_INIT:
                return STATE_INIT_STRING;
            case STATE_RUNNABLE:
                return STATE_RUNNABLE_STRING;
            case STATE_RUNNING:
                return STATE_RUNNING_STRING;
            case STATE_BLOCKED:
                return STATE_BLOCKED_STRING;
            case STATE_COMPLETE:
                return STATE_COMPLETE_STRING;
            default:
                return STATE_UNKNOWN_STRING;
        }
    }

    // XXX: Might we need an inheritCondition in addition to addCondition?
    //      Would this make the logic easier here and in the conditions?

    public boolean addCondition(STAXCondition condition)
    {
        synchronized (fConditionSet)
        { return fConditionSet.add(condition); }
    }

    public boolean removeCondition(STAXCondition condition)
    {
        synchronized (fConditionSet)
        { return fConditionSet.remove(condition); }
    }

    public void visitConditions(STAXVisitor visitor)
    {
        synchronized (fConditionSet)
        {
            Iterator iter = fConditionSet.iterator();

            while (iter.hasNext())
                visitor.visit(iter.next(), iter);
        }
    }

    //
    // Execution functions
    //

    public void pushAction(STAXAction action)
    {
        synchronized (fActionStack)
        {
            fActionStack.addLast(action);

            if (fBreakpointFirstFunction &&
                (action instanceof STAXFunctionAction))
            {
                fBreakpointCondition = true;
                fBreakpointFirstFunction = false;
            }

            boolean pushBreakpoint = fBreakpointCondition;

            // If we are not already stepping through the action stack, and
            // there are breakpoints set, check to see if this action matches
            // a breakpoint function or line
            if (!pushBreakpoint && !(fJob.breakpointsEmpty()))
            {
                // Check for a function breakpoint
                if ((action instanceof STAXFunctionAction) &&
                    (fJob.isBreakpointFunction(
                    ((STAXFunctionAction)action).getName())))
                {
                    pushBreakpoint = true;
                }
                // Check for a line breakpoint
                else if (fJob.isBreakpointLine(
                         ((STAXActionDefaultImpl)action).getLineNumber(),
                         ((STAXActionDefaultImpl)action).getXmlFile(),
                         ((STAXActionDefaultImpl)action).getXmlMachine()))
                {
                    pushBreakpoint = true;
                }
            }

            if (pushBreakpoint)
            {
                if (action instanceof STAXBreakpointAction)
                {
                    // This is a breakpoint element, so we don't need to
                    // add a new STAXBreakpointAction
                }
                else
                {
                    if (fBreakpointActionFactory == null)
                    {
                        fBreakpointActionFactory =
                            (STAXBreakpointActionFactory)
                            fJob.getSTAX().getActionFactory("breakpoint");
                    }

                    // Create a new STAXBreakpointAction and add it to the
                    // end of the action stack, so that it is executed before
                    // the action that was previously added to the end of the
                    // action stack
                    STAXAction stepBreakpointAction =
                        fBreakpointActionFactory.createStepBreakpoint(action);

                    fActionStack.addLast(stepBreakpointAction);
                }
            }
        }
    }

    public void popAction()
    {
        synchronized (fActionStack)
        {
            STAXAction lastAction = (STAXAction)fActionStack.getLast();

            fActionStack.removeLast();

            if ((lastAction instanceof STAXBreakpointAction) &&
                fStepOverCondition)
            {
                // If we are popping off a STAXBreakpointAction after a STEP
                // OVER request, then set the fStepOverIndex to be 1 less than
                // the current action stack size.  The next breakpoint should
                // be added after the action stack size equals the
                // fStepOverIndex.  This ensures that the last action in the
                // action stack, and any sub-actions of the last action in the
                // action stack, are executed
                fStepOverIndex = fActionStack.size() - 1;
            }

            if (fStepOverIndex == fActionStack.size())
            {
                if (!(lastAction instanceof STAXCallAction))
                {
                    // Only do this if the lastAction is not a STAXCallAction.
                    // This is because the STAXCallAction is popped from the
                    // action stack before the STAXFunctionAction is added.
                    // In this scenario, we want to add the breakpoint after
                    // the function has completed (i.e. when the fStepOverIndex
                    // equals the fActionStack size, and the lastAction is not
                    // a STAXCallAction)
                    fBreakpointCondition = true;
                    fStepOverIndex = -1;
                }
            }
        }
    }

    public void visitActions(STAXVisitor visitor)
    {
        synchronized (fActionStack)
        {
            Iterator iter = fActionStack.iterator();
            int i = 0;

            while (iter.hasNext())
            {
                if ((fThreadNumber == 1) && (i < 2))
                {
                    // Ignore the first two actions in the action stack for
                    // thread 1 as they are internal to the STAX service and
                    // don't need to be provided in the call stack:
                    // 1) STAXBlockAction for the "main" block
                    // 2) STAXSequenceAction for the sequence in the "main"
                    //    block
                    i++;
                    iter.next();
                    continue;
                }

                visitor.visit(iter.next(), iter);
            }
        }
    }

    public void schedule()
    {
        boolean initState = false;

        synchronized (fConditionSet)
        {
            if (fState == STATE_INIT)
            {
                initState = true;
                fState = STATE_BLOCKED;
            }
            
            if (fState == STATE_BLOCKED)
            {
                fState = STATE_RUNNABLE;
                fJob.getSTAX().getThreadQueue().add(this);
            }
        }

        // Did it this way so that generating a thread start event would not
        // be part of the synchronized (fConditionSet) block above.

        if (initState)
        {
            // Generate a thread start event

            HashMap eventMap = new HashMap();
            eventMap.put("type", "thread");
            eventMap.put("id", String.valueOf(fThreadNumber));
            eventMap.put("status", "start");

            if (fParent != null)
            {
                eventMap.put("parent",
                             String.valueOf(fParent.getThreadNumber()));
                eventMap.put("parentHierarchy", fThreadParentHierarchy);
            }

            List callStack = getCallStack();

            if (callStack.size() > 0)
            {
                eventMap.put("detailText",
                             (String)callStack.get(callStack.size() - 1));
            }
            
            fJob.generateEvent(STAXThread.THREAD, eventMap);
        }
    }

    public void execute()
    {
        int numActions = 0;

        fState = STATE_RUNNING;

        while (true)
        {
            if (++numActions == MAX_NON_BLOCKING_ACTIONS)
            {
                synchronized (fConditionSet)
                {
                    fState = STATE_BLOCKED;
                    schedule();
                    return;
                }
            }

            if (fActionStack.size() == 0)
            {
                synchronized (fChildThreadSet)
                {
                    synchronized (fConditionSet)
                    {
                        if (fChildThreadSet.size() != 0)
                        {
                            fState = STATE_BLOCKED;
                            return;
                        }
                    }
                }

                fState = STATE_COMPLETE;

                if (fParent == null)
                {
                    try
                    {
                        fJob.setCompletionStatus(pyIntEval("STAXBlockRC"));
                    }
                    catch (STAXPythonEvaluationException e)
                    {
                        fJob.setCompletionStatus(STAXJob.ABNORMAL_STATUS);

                        fJob.log(STAXJob.JOB_LOG, "error", "Error evaluating" +
                                 " STAXBlockRC variable when setting job " +
                                 "completion status");
                    }
                }

                synchronized (fConditionSet)
                {
                    boolean addedParentCondition = false;
                    Iterator iter = fConditionSet.iterator();

                    while (iter.hasNext())
                    {
                        STAXCondition cond = (STAXCondition)iter.next();

                        // XXX: We need to get the parent running again so
                        //      that it may kill any children threads if
                        //      necessary
                        // XXX: Is it safe to schedule the parent?
                        // XXX: Should addCondition call schedule()?

                        if ((fParent == null) && cond.isInheritable())
                        {
                            fJob.setCompletionStatus(STAXJob.ABNORMAL_STATUS);

                            String className = STAXUtil.getShortClassName(
                                cond.getClass().getName(), "Condition");

                            StringBuffer errorMsg = new StringBuffer(
                                "Unhandled \"");

                            errorMsg.append(className).append(
                                "\" condition found at end of job.");

                            if ((cond.getSource() != null) &&
                                (cond.getSource().length() > 0))
                            {
                                errorMsg.append("\nSource: ").append(
                                    cond.getSource());

                                if (className.equals("Exception"))
                                {
                                    errorMsg.append("\nInfo: ").append(
                                        ((STAXExceptionCondition)
                                        cond).getData());

                                    List stackTrace = ((STAXExceptionCondition)
                                        cond).getStackTrace();

                                    errorMsg.append(
                                        "\n\n===== Stack Trace =====\n\n" +
                                        STAFMarshallingContext.formatObject(
                                        stackTrace));
                                }
                            }

                            fJob.log(STAXJob.JOB_LOG, "error",
                                     errorMsg.toString());
                        }
                        else if (cond.isInheritable())
                        {
                            fParent.addCondition(cond);
                            addedParentCondition = true;
                        }
                    }

                    if (fBreakpointCondition && (fParent != null))
                    {
                        fParent.setBreakpointCondition(true);
                    }

                    if (addedParentCondition) fParent.schedule();
                }

                if (STAX_DEBUG_THREAD)
                {
                    System.out.println("Thread #" + fThreadNumber +
                                       ": complete");
                }

                HashMap threadMap = new HashMap();
                threadMap.put("type", "thread");
                threadMap.put("id", String.valueOf(fThreadNumber));
                threadMap.put("status", "stop");

                fJob.generateEvent(STAXThread.THREAD, threadMap);

                while (!fCompletionNotifiees.isEmpty())
                {
                    STAXThreadCompleteListener listener =
                        (STAXThreadCompleteListener)
                        fCompletionNotifiees.removeFirst();

                    if (listener != null)
                        listener.threadComplete(this, fCompletionCode);
                }

                return;
            }

            STAXAction action = (STAXAction)fActionStack.getLast();
            STAXCondition currCondition = null;

            synchronized (fConditionSet)
            {
                if (fConditionSet.size() != 0)
                {
                    currCondition = (STAXCondition)fConditionSet.first();

                }

                if ((currCondition != null) &&
                    ((currCondition instanceof STAXHoldThreadCondition) ||
                     (currCondition instanceof STAXHardHoldThreadCondition)))
                {
                    if (STAX_DEBUG_THREAD)
                    {
                        String condName = currCondition.getClass().getName();
                        condName = condName.substring(
                                   condName.lastIndexOf(".") + 1);

                        System.out.println("Thread #" + fThreadNumber +
                                           ": blocking due to " + condName);
                    }

                    fState = STATE_BLOCKED;
                    return;
                }
            }

            if (currCondition == null)
            {
                if (STAX_DEBUG_THREAD)
                {
                    String actionName = action.getClass().getName();
                    actionName = actionName.substring(
                                 actionName.lastIndexOf(".") + 1);

                    System.out.println("Thread #" + fThreadNumber +
                                       ": calling " + actionName);
                }

                action.execute(this);
            }
            else
            {
                if (STAX_DEBUG_THREAD)
                {
                    String actionName = action.getClass().getName();
                    actionName = actionName.substring(
                                 actionName.lastIndexOf(".") + 1);
                    String condName = currCondition.getClass().getName();
                    condName = condName.substring(
                               condName.lastIndexOf(".") + 1);

                    System.out.println("Thread #" + fThreadNumber +
                                       ": calling " + actionName +
                                       ", handling " + condName);
                }

                action.handleCondition(this, currCondition);
            }

        }  // end while
    }

    public void terminate(int termCode)
    {
        synchronized (fConditionSet)
        {
            fCompletionCode = termCode;
            addCondition(new STAXTerminateThreadCondition("Thread"));
            schedule();
        }
    }

    // STAXThreadCompleteListener method

    public void threadComplete(STAXThread thread, int endCode)
    {
        synchronized(fChildThreadSet)
        {
            Integer threadID = thread.getThreadNumberAsInteger();
            fChildThreadSet.remove(threadID);
            thread.getJob().removeThread(threadID);

            if (fChildThreadSet.size() == 0)
                schedule();
        }
    }


    // This class is used to sort the conditions in the condition set

    class STAXConditionComparator implements Comparator
    {
        public int compare(Object o1, Object o2)
        {
            STAXCondition c1 = (STAXCondition)o1;
            STAXCondition c2 = (STAXCondition)o2;

            if (c1.getPriority() == c2.getPriority())
            {
                if (o1.hashCode() == o2.hashCode()) return 0;
                else if (o1.hashCode() < o2.hashCode()) return -1;
            }
            else if (c1.getPriority() < c2.getPriority()) return -1;

            return 1;
        }
    }

    public void setBreakpointCondition(boolean breakpointCondition)
    {
        this.fBreakpointCondition = breakpointCondition;
    }

    public boolean getBreakpointCondition()
    {
        return fBreakpointCondition;
    }

    public void setStepOverCondition(boolean stepOverCondition)
    {
        this.fStepOverCondition = stepOverCondition;
    }

    public void setBreakpointFirstFunction(boolean breakpointFirstFunction)
    {
        this.fBreakpointFirstFunction = breakpointFirstFunction;
    }

    private boolean fBreakpointCondition = false;
    private boolean fStepOverCondition = false;
    private boolean fBreakpointFirstFunction = false;
    private int fStepOverIndex = -1;
    private int fState = STATE_INIT;
    private int fCompletionCode = THREAD_END_OK;
    private String fTerminationBlock = new String();
    private STAXJob fJob = null;
    private int fThreadNumber = 0;
    private STAXThread fParent = null;
    private HashMap fSignalHandlerMap = new HashMap();
    private STAXPythonInterpreter fPythonInterpreter;
    private LinkedList fActionStack = new LinkedList();
    private LinkedList fSignalStack = new LinkedList();
    private TreeSet fHeldBlocksSet = new TreeSet();
    private TreeSet fConditionSet = new TreeSet(new STAXConditionComparator());
    private LinkedList fCompletionNotifiees = new LinkedList();
    private Set fChildThreadSet = new LinkedHashSet();
    private STAXTimestamp fStartTimestamp;
    private STAXBreakpointActionFactory fBreakpointActionFactory = null;
    private String fThreadParentHierarchy = "";
}
