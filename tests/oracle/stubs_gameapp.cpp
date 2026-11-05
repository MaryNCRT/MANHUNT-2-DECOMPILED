// stubs_gameapp.cpp — GameApp methods whose real versions need code that is
// not decompiled yet. The tests never send the events that reach them; if one
// is reached anyway the run stops with a message instead of passing silently.
#include "../../decomp/core/GameApp.h"

#include <cstdio>
#include <cstdlib>

static int Unreachable(const char* what)
{
    std::fprintf(stderr, "\nORACLE STUB: %s reached; it has no test stand-in.\n", what);
    std::exit(3);
}

int GameApp::Initialise(void*) { return Unreachable("GameApp::Initialise"); }
int GameApp::Shutdown() { return Unreachable("GameApp::Shutdown"); }
