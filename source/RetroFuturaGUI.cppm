/* The RetroFuturaGUI module, built in place of the plain headers when RETROFUTURAGUI_MODULES is on. Everything from
   outside the library comes first, outside the module; the library's own headers follow inside it, where RFG_EXPORT
   marks what an importer sees. Importers include config.hpp for the number types and the macros, then
   import RetroFuturaGUI
*/
module;
#include "ModuleExternals.hpp"

export module RetroFuturaGUI;

#undef RFG_EXPORT
#define RFG_EXPORT export

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 5244) //the library's headers are included inside the module on purpose
#endif
#include "IncludeHelper.hpp"
#include "Transition.hpp"
#ifdef _MSC_VER
    #pragma warning(pop)
#endif