# mac.pro
# Since 7/30/2011
mac {

TEMPLATE = subdirs
include(qtmac/qtmac.pri)
include(qtstep/qtstep.pri)
include(qtvlc/qtvlc.pri)
#include(VLCKit/VLCKit.pri)
SUBDIRS = \
  macdeploy \
  macdeployqt \
  VLCKit

}
# EOF
