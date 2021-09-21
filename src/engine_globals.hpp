#ifdef __GLOBALS_HEADERS__
/* Put all includes or defiitions required for your Global types here */

#else

//#define 
//
/** 
 * here you can define your globals via REGISTER_GLOBAL 
 *
 * Usage: REGISTER_GLOBAL(type, name, initialvalue)
 * - type = datatype (int, float, void*, etc....)
 * - name = of the global variable
 * - value = with which the global variable will be initialized
 *
 * the following global names cant be registered;
 * - game   ==> contains a reference to the currently running engine
 * - world
 */

/* An example */
REGISTER_GLOBAL(int, score, 0)

#endif


