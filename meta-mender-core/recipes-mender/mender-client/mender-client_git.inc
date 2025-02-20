require mender-client.inc

DEPENDS = "xz openssl"
RDEPENDS_${PN} = "liblzma openssl"

# The revision listed below is not really important, it's just a way to avoid
# network probing during parsing if we are not gonna build the git version
# anyway. If git version is enabled, the AUTOREV will be chosen instead of the
# SHA.
def mender_autorev_if_git_version(d):
    version = d.getVar("PREFERRED_VERSION")
    if version is None or version == "":
        version = d.getVar("PREFERRED_VERSION_%s" % d.getVar('PN'))
    if not d.getVar("EXTERNALSRC") and version is not None and "git" in version:
        return d.getVar("AUTOREV")
    else:
        return "f6ffa190892202263fdb75975059fbb201adab6a"
SRCREV ?= '${@mender_autorev_if_git_version(d)}'

# Force the version of master and feature-branch packages to compare very highly
# to other versions.
PKGV = "${@'999+${PV}' if any(['${PV}'.startswith(p) for p in ['master', 'feature-']]) else '${PV}'}"

def mender_branch_from_preferred_version(d):
    import re
    version = d.getVar("PREFERRED_VERSION")
    if version is None or version == "":
        version = d.getVar("PREFERRED_VERSION_%s" % d.getVar('PN'))
    if version is None:
        version = ""
    match = re.match(r"^[0-9]+\.[0-9]+\.", version)
    if match is not None:
        # If the preferred version is some kind of version, use the branch name
        # for that one (1.0.x style).
        return match.group(0) + "x"
    else:
        # Else return master as branch.
        return "master"
MENDER_BRANCH = "${@mender_branch_from_preferred_version(d)}"

def mender_version_from_preferred_version(d, srcpv):
    pref_version = d.getVar("PREFERRED_VERSION")
    if pref_version is None:
        pref_version = d.getVar("PREFERRED_VERSION_%s" % d.getVar("PN"))
    if pref_version is not None and pref_version.find("-git") >= 0:
        # If "-git" is in the version, remove it along with any suffix it has,
        # and then readd it with commit SHA.
        return "%s-git%s" % (pref_version[0:pref_version.index("-git")], srcpv)
    elif pref_version is not None and pref_version.find("-build") >= 0:
        # If "-build" is in the version, use the version as is. This means that
        # we can build tags with "-build" in them from this recipe, but not
        # final tags, which will need their own recipe.
        return pref_version
    else:
        # Else return the default "master-git".
        return "master-git%s" % srcpv

SRC_URI = "git://github.com/mendersoftware/mender;protocol=https;branch=${MENDER_BRANCH}"

# DO NOT change the checksum here without make sure that ALL licenses (including
# dependencies) are included in the LICENSE variable below. Note that for
# releases, we must check the LIC_FILES_CHKSUM.sha256 file, not the LICENSE
# file.
def mender_license(branch):
    import re
    if branch.startswith("1."):
        return {
                   "license": "Apache-2.0 & BSD-2-Clause & BSD-3-Clause & ISC & MIT & OLDAP-2.8",
        }
    elif re.match(r"2\.[0-3]\.x", branch) is not None:
        return {
                   "license": "Apache-2.0 & BSD-2-Clause & BSD-3-Clause & ISC & MIT & OLDAP-2.8",
        }
    else:
        return {
                   "license": "Apache-2.0 & BSD-2-Clause & BSD-3-Clause & ISC & MIT & OLDAP-2.8 & OpenSSL",
        }
LIC_FILES_CHKSUM = "file://src/github.com/mendersoftware/mender/LICENSE;md5=4cd0c347af5bce5ccf3b3d5439a2ea87"
LICENSE = "${@mender_license(d.getVar('MENDER_BRANCH'))['license']}"

# Downprioritize this recipe in version selections.
DEFAULT_PREFERENCE = "-1"
