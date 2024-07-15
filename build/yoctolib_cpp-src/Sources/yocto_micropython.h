/*********************************************************************
 *
 *  $Id: svn_id $
 *
 *  Declares yFindMicroPython(), the high-level API for MicroPython functions
 *
 *  - - - - - - - - - License information: - - - - - - - - -
 *
 *  Copyright (C) 2011 and beyond by Yoctopuce Sarl, Switzerland.
 *
 *  Yoctopuce Sarl (hereafter Licensor) grants to you a perpetual
 *  non-exclusive license to use, modify, copy and integrate this
 *  file into your software for the sole purpose of interfacing
 *  with Yoctopuce products.
 *
 *  You may reproduce and distribute copies of this file in
 *  source or object form, as long as the sole purpose of this
 *  code is to interface with Yoctopuce products. You must retain
 *  this notice in the distributed source file.
 *
 *  You should refer to Yoctopuce General Terms and Conditions
 *  for additional information regarding your rights and
 *  obligations.
 *
 *  THE SOFTWARE AND DOCUMENTATION ARE PROVIDED 'AS IS' WITHOUT
 *  WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING
 *  WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS
 *  FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO
 *  EVENT SHALL LICENSOR BE LIABLE FOR ANY INCIDENTAL, SPECIAL,
 *  INDIRECT OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA,
 *  COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR
 *  SERVICES, ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT
 *  LIMITED TO ANY DEFENSE THEREOF), ANY CLAIMS FOR INDEMNITY OR
 *  CONTRIBUTION, OR OTHER SIMILAR COSTS, WHETHER ASSERTED ON THE
 *  BASIS OF CONTRACT, TORT (INCLUDING NEGLIGENCE), BREACH OF
 *  WARRANTY, OR OTHERWISE.
 *
 *********************************************************************/


#ifndef YOCTO_MICROPYTHON_H
#define YOCTO_MICROPYTHON_H

#include <cfloat>
#include <cmath>

#include "yocto_api.h"

#ifdef YOCTOLIB_NAMESPACE
namespace YOCTOLIB_NAMESPACE
{
#endif

//--- (YMicroPython return codes)
//--- (end of YMicroPython return codes)
//--- (YMicroPython yapiwrapper)
//--- (end of YMicroPython yapiwrapper)
//--- (YMicroPython definitions)
class YMicroPython; // forward declaration

typedef void (*YMicroPythonValueCallback)(YMicroPython *func, const string& functionValue);
typedef void (*YMicroPythonLogCallback)(YMicroPython *obj, const string& logline);
#define Y_LASTMSG_INVALID               (YAPI_INVALID_STRING)
#define Y_CURRENTSCRIPT_INVALID         (YAPI_INVALID_STRING)
#define Y_STARTUPSCRIPT_INVALID         (YAPI_INVALID_STRING)
#define Y_COMMAND_INVALID               (YAPI_INVALID_STRING)
//--- (end of YMicroPython definitions)

//--- (YMicroPython declaration)
/**
 * YMicroPython Class: MicroPython interpreter control interface
 *
 * The YMicroPython class provides control of the MicroPython interpreter
 * that can be found on some Yoctopuce devices.
 */
class YOCTO_CLASS_EXPORT YMicroPython: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YMicroPython declaration)
protected:
    static void yInternalEventCallback(YMicroPython *obj, const string& value);

    //--- (YMicroPython attributes)
    // Attributes (function value cache)
    string          _lastMsg;
    string          _currentScript;
    string          _startupScript;
    string          _command;
    YMicroPythonValueCallback _valueCallbackMicroPython;
    YMicroPythonLogCallback _logCallback;
    bool            _isFirstCb;
    int             _prevCbPos;
    int             _logPos;
    string          _prevPartialLog;

    friend YMicroPython *yFindMicroPython(const string& func);
    friend YMicroPython *yFirstMicroPython(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject *json_val);

    // Constructor is protected, use yFindMicroPython factory function to instantiate
    YMicroPython(const string& func);
    //--- (end of YMicroPython attributes)

public:
    virtual ~YMicroPython();
    //--- (YMicroPython accessors declaration)

    static const string LASTMSG_INVALID;
    static const string CURRENTSCRIPT_INVALID;
    static const string STARTUPSCRIPT_INVALID;
    static const string COMMAND_INVALID;

    /**
     * Returns the last message produced by a python script.
     *
     * @return a string corresponding to the last message produced by a python script
     *
     * On failure, throws an exception or returns YMicroPython::LASTMSG_INVALID.
     */
    string              get_lastMsg(void);

    inline string       lastMsg(void)
    { return this->get_lastMsg(); }

    /**
     * Returns the name of currently active script, if any.
     *
     * @return a string corresponding to the name of currently active script, if any
     *
     * On failure, throws an exception or returns YMicroPython::CURRENTSCRIPT_INVALID.
     */
    string              get_currentScript(void);

    inline string       currentScript(void)
    { return this->get_currentScript(); }

    /**
     * Stops current running script, and/or selects a script to run immediately in a
     * fresh new environment. If the MicroPython interpreter is busy running a script,
     * this function will abort it immediately and reset the execution environment.
     * If a non-empty string is given as argument, the new script will be started.
     *
     * @param newval : a string
     *
     * @return YAPI::SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_currentScript(const string& newval);
    inline int      setCurrentScript(const string& newval)
    { return this->set_currentScript(newval); }

    /**
     * Returns the name of the script to run when the device is powered on.
     *
     * @return a string corresponding to the name of the script to run when the device is powered on
     *
     * On failure, throws an exception or returns YMicroPython::STARTUPSCRIPT_INVALID.
     */
    string              get_startupScript(void);

    inline string       startupScript(void)
    { return this->get_startupScript(); }

    /**
     * Changes the script to run when the device is powered on.
     * Remember to call the saveToFlash() method of the module if the
     * modification must be kept.
     *
     * @param newval : a string corresponding to the script to run when the device is powered on
     *
     * @return YAPI::SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_startupScript(const string& newval);
    inline int      setStartupScript(const string& newval)
    { return this->set_startupScript(newval); }

    string              get_command(void);

    inline string       command(void)
    { return this->get_command(); }

    int             set_command(const string& newval);
    inline int      setCommand(const string& newval)
    { return this->set_command(newval); }

    /**
     * Retrieves a MicroPython interpreter for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the MicroPython interpreter is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method isOnline() to test if the MicroPython interpreter is
     * indeed online at a given time. In case of ambiguity when looking for
     * a MicroPython interpreter by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the MicroPython interpreter, for instance
     *         MyDevice.microPython.
     *
     * @return a YMicroPython object allowing you to drive the MicroPython interpreter.
     */
    static YMicroPython* FindMicroPython(string func);

    /**
     * Registers the callback function that is invoked on every change of advertised value.
     * The callback is invoked only during the execution of ySleep or yHandleEvents.
     * This provides control over the time when the callback is triggered. For good responsiveness, remember to call
     * one of these two functions periodically. To unregister a callback, pass a NULL pointer as argument.
     *
     * @param callback : the callback function to call, or a NULL pointer. The callback function should take two
     *         arguments: the function object of which the value has changed, and the character string describing
     *         the new advertised value.
     * @noreturn
     */
    virtual int         registerValueCallback(YMicroPythonValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);

    /**
     * Submit MicroPython code for execution in the interpreter.
     * If the MicroPython interpreter is busy, this function will
     * block until it becomes available. The code is then uploaded,
     * compiled and executed on the fly, without beeing stored on the device filesystem.
     *
     * There is no implicit reset of the MicroPython interpreter with
     * this function. Use method reset() if you need to start
     * from a fresh environment to run your code.
     *
     * Note that although MicroPython is mostly compatible with recent Python 3.x
     * interpreters, the limited ressources on the device impose some restrictions,
     * in particular regarding the libraries that can be used. Please refer to
     * the documentation for more details.
     *
     * @param codeName : name of the code file (used for error reporting only)
     * @param mpyCode : MicroPython code to compile and execute
     *
     * @return YAPI::SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         eval(string codeName,string mpyCode);

    /**
     * Stops current execution, and reset the MicroPython interpreter to initial state.
     * All global variables are cleared, and all imports are forgotten.
     *
     * @return YAPI::SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         reset(void);

    /**
     * Returns a string with last logs of the MicroPython interpreter.
     * This method return only logs that are still in the module.
     *
     * @return a string with last MicroPython logs.
     *         On failure, throws an exception or returns  YAPI::INVALID_STRING.
     */
    virtual string      get_lastLogs(void);

    /**
     * Registers a device log callback function. This callback will be called each time
     * microPython sends a new log message.
     *
     * @param callback : the callback function to invoke, or a NULL pointer.
     *         The callback function should take two arguments:
     *         the module object that emitted the log message,
     *         and the character string containing the log.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         registerLogCallback(YMicroPythonLogCallback callback);

    virtual YMicroPythonLogCallback_callback* get_logCallback(void);

    virtual int         _internalEventHandler(string cbVal);


    inline static YMicroPython *Find(string func)
    { return YMicroPython::FindMicroPython(func); }

    /**
     * Continues the enumeration of MicroPython interpreters started using yFirstMicroPython().
     * Caution: You can't make any assumption about the returned MicroPython interpreters order.
     * If you want to find a specific a MicroPython interpreter, use MicroPython.findMicroPython()
     * and a hardwareID or a logical name.
     *
     * @return a pointer to a YMicroPython object, corresponding to
     *         a MicroPython interpreter currently online, or a NULL pointer
     *         if there are no more MicroPython interpreters to enumerate.
     */
           YMicroPython    *nextMicroPython(void);
    inline YMicroPython    *next(void)
    { return this->nextMicroPython();}

    /**
     * Starts the enumeration of MicroPython interpreters currently accessible.
     * Use the method YMicroPython::nextMicroPython() to iterate on
     * next MicroPython interpreters.
     *
     * @return a pointer to a YMicroPython object, corresponding to
     *         the first MicroPython interpreter currently online, or a NULL pointer
     *         if there are none.
     */
           static YMicroPython *FirstMicroPython(void);
    inline static YMicroPython *First(void)
    { return YMicroPython::FirstMicroPython();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YMicroPython accessors declaration)
};

//--- (YMicroPython functions declaration)

/**
 * Retrieves a MicroPython interpreter for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the MicroPython interpreter is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method isOnline() to test if the MicroPython interpreter is
 * indeed online at a given time. In case of ambiguity when looking for
 * a MicroPython interpreter by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the MicroPython interpreter, for instance
 *         MyDevice.microPython.
 *
 * @return a YMicroPython object allowing you to drive the MicroPython interpreter.
 */
inline YMicroPython *yFindMicroPython(const string& func)
{ return YMicroPython::FindMicroPython(func);}
/**
 * Starts the enumeration of MicroPython interpreters currently accessible.
 * Use the method YMicroPython::nextMicroPython() to iterate on
 * next MicroPython interpreters.
 *
 * @return a pointer to a YMicroPython object, corresponding to
 *         the first MicroPython interpreter currently online, or a NULL pointer
 *         if there are none.
 */
inline YMicroPython *yFirstMicroPython(void)
{ return YMicroPython::FirstMicroPython();}

//--- (end of YMicroPython functions declaration)

#ifdef YOCTOLIB_NAMESPACE
// end of namespace definition
}
#endif

#endif
