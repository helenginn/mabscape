set path=./,libsrc/**,libgui/**,libinfo/**,doc/

command! Tags !ctags -R libgui/* libsrc/*
command! Ninja :wa|!ninja -C build/current
command! Dinja :wa|!ninja -C build/debug
command! Make1 !cd libgui/qtgui; make;

command! Doxy !doxygen Doxyfile



