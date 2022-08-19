#ifndef C_BRIDGE_H
#define C_BRIDGE_H

#ifdef _MSC_VER
#if defined(c_bridge_EXPORTS)
#define C_BRIDGE_API __declspec(dllexport)
#define C_BRIDGE_TEMPLATE_EXT
#else
#define C_BRIDGE_API __declspec(dllimport)
#define C_BRIDGE_TEMPLATE_EXT extern
#endif
#else
#define C_BRIDGE_API
#define C_BRIDGE_TEMPLATE_EXT
#endif

C_BRIDGE_API double cos_wrap(double);

#ifndef C_BRIDGE_IMPL
#define cos cos_wrap
#endif

#endif
