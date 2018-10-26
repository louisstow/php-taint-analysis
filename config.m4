PHP_ARG_ENABLE(phd, whether to enable PHD support,
[ --enable-phd   Enable PHD support])

if test "$PHP_PHD" = "yes"; then
	AC_DEFINE(PHD, 1, [Whether you have PHD])
	PHP_NEW_EXTENSION(phd, src/phd.c src/util.c src/tree.c src/taint.c, $ext_shared)
fi
