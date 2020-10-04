SUBDIRS = images meshes gradients xaos

AM_LDFLAGS = -export-symbols $(top_srcdir)/lemuria.sym

xaos_libadd = xaos/src/filter/libfilter.la \
xaos/src/engine/libengine.la \
xaos/src/ui-hlp/libui_hlp.la \
xaos/src/util/libutil.la

lib_LTLIBRARIES = liblemuria.la

# liblemuria_la_SOURCES = lemuria_plugin.c

liblemuria_la_LIBADD = -L/usr/X11R6/lib @GLU_LIBS@ @GL_LIBS@ -lpthread $(xaos_libadd) -lpthread -lXinerama

liblemuria_la_SOURCES = effect.c \
analyis.c \
archaic.c \
blur.c \
background.c \
boioioing.c \
deepsea.c \
drive.c \
font.c \
goom.c \
hyperbubble.c \
lemuria.c \
icosphere.c \
lemuria_geometry.c \
lemuria_teapot.c \
lineobjects.c \
bubbles.c \
manta.c \
monolith.c \
mountains.c \
object.c \
oscilloscope.c \
oszi3d.c \
platon.c \
psycho.c \
cube.c \
fish.c \
fountain.c \
lines.c \
light.c \
material.c \
offset.c \
particle.c \
perspective.c \
random.c \
range.c \
satellite.c \
scale.c \
sputnik.c \
squares.c \
superellipse.c \
swarm.c \
teapot.c \
tentacle.c \
texture.c \
timer.c \
tube.c \
tunnel.c \
ufo.c \
fft.c \
starfield.c \
rotate.c \
vectorscope.c \
xaos.c \
obj_bubbles.c \
seaplant.c

EXTRA_liblemuria_la_SOURCES = font.incl
EXTRA_DIST = v4l.c

