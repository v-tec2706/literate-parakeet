Format: 3.0 (quilt)
Source: zeroc-ice
Binary: libzeroc-ice-dev, libzeroc-ice3.7-java, libzeroc-ice-java, libzeroc-ice3.7, libzeroc-icestorm3.7, php-zeroc-ice, python-zeroc-ice, python3-zeroc-ice, zeroc-glacier2, zeroc-ice-all-dev, zeroc-ice-all-runtime, zeroc-ice-compilers, zeroc-ice-slice, zeroc-ice-utils, zeroc-icegridgui, zeroc-ice-utils-java, zeroc-icebox, zeroc-icegrid, zeroc-icepatch2, zeroc-icebridge
Architecture: any all
Version: 3.7.2-1
Maintainer: José Gutiérrez de la Concha <jose@zeroc.com>
Uploaders: Ondřej Surý <ondrej@debian.org>
Homepage: https://zeroc.com
Standards-Version: 4.1.3
Vcs-Browser: https://github.com/zeroc-ice/ice-debian-packaging.git
Vcs-Git: https://github.com/zeroc-ice/ice-debian-packaging.git
Build-Depends: debhelper (>= 9), dh-exec, dh-php (>= 0.20), dh-python, dh-systemd (>= 1.3), javahelper, libbluetooth-dev [!hurd-i386 !kfreebsd-i386 !kfreebsd-amd64], libbz2-dev, libdbus-1-dev [!hurd-i386 !kfreebsd-i386 !kfreebsd-amd64], libexpat1-dev, liblmdb-dev, libmcpp-dev, libssl-dev, libsystemd-dev, locales-all, maven-repo-helper (>= 1.8), openssl, php-all-dev, php-cli, python, python-dev, python-passlib, python-setuptools, python3, python3-dev, python3-setuptools
Build-Depends-Indep: default-jdk, gradle (>= 2), gradle-ice-builder-plugin (>= 1.3.13), libjgoodies-forms-java (>= 1.6.0), libjgoodies-looks-java (>= 2.5.2)
Package-List:
 libzeroc-ice-dev deb libdevel optional arch=any
 libzeroc-ice-java deb oldlibs optional arch=all
 libzeroc-ice3.7 deb libs optional arch=any
 libzeroc-ice3.7-java deb java optional arch=all
 libzeroc-icestorm3.7 deb net optional arch=any
 php-zeroc-ice deb php optional arch=any
 python-zeroc-ice deb python optional arch=any
 python3-zeroc-ice deb python optional arch=any
 zeroc-glacier2 deb net optional arch=any
 zeroc-ice-all-dev deb devel optional arch=all
 zeroc-ice-all-runtime deb net optional arch=all
 zeroc-ice-compilers deb devel optional arch=any
 zeroc-ice-slice deb devel optional arch=all
 zeroc-ice-utils deb admin optional arch=any
 zeroc-ice-utils-java deb oldlibs optional arch=all
 zeroc-icebox deb net optional arch=any
 zeroc-icebridge deb net optional arch=any
 zeroc-icegrid deb net optional arch=any
 zeroc-icegridgui deb admin optional arch=all
 zeroc-icepatch2 deb net optional arch=any
Checksums-Sha1:
 91aec6d9b43f3570aa2f408e447ab5f2c4844bbf 9351744 zeroc-ice_3.7.2.orig.tar.gz
 10f151d499a00385254ee3a343e73d5d65c575b8 30945 zeroc-ice_3.7.2-1.debian.tar.gz
Checksums-Sha256:
 fdab8cc345c7ed315ffe229423e95e5e183aa00a036093e98081f314dbff8b72 9351744 zeroc-ice_3.7.2.orig.tar.gz
 93ea436f41756f30dc818cf4768e35e7055dbfad9101c08614cb30e4d6b55026 30945 zeroc-ice_3.7.2-1.debian.tar.gz
Files:
 f78f23eda3fd7e2c04a04dd65715991a 9351744 zeroc-ice_3.7.2.orig.tar.gz
 4acfb54d2949b21ca7b70e1895e63d75 30945 zeroc-ice_3.7.2-1.debian.tar.gz
