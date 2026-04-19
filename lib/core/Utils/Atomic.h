#pragma once

#ifdef __AVR__

    #include <util/atomic.h>
    #define ATOMIC_SECTION ATOMIC_BLOCK(ATOMIC_RESTORESTATE)

#else

    // No PC não precisa de proteção
    #define ATOMIC_SECTION for (bool _once = true; _once; _once = false)

#endif