set path=./,libsrc/**,kinsrc/**,

command! Tags !ctags -R kinsrc/* libsrc/*
command! Ninja :wa|!ninja -C build/current
command! Dinja :wa|!ninja -C build/debug
command! Make1 !cd libgui/qtgui; make;

command! Doxy !doxygen Doxyfile



