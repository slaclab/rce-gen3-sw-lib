#!/bin/env python
#-*-Mode: Python;-*-
# @file
# @brief Print the page ID-form URLs for all children of the
# "unused pages" page.
# @verbatim
#                               Copyright 2014
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
# @endverbatim
#
# @par Facility:
# DAT
#
# @author
# Stephen Tether <tether@slac.stanford.edu>
#
# @par Date created:
# 2014/03/28
#
# @par Last commit:
# \$Date: 2014-04-09 22:43:44 -0700 (Wed, 09 Apr 2014) $ by \$Author: tether $.
#
# @par Revision number:
# \$Revision: 3144 $
#
# @par Location in repository:
# \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/hwdb/src/linux/listblanks.py $
#
# @par Credits:
# SLAC


# Python library.
import getpass, sys

# HWDB code.
import cli, page

def main():
    user = getpass.getuser()
    rc = cli.login(user, getpass.getpass("Confluence password for %s: " % user), page.SPACE_KEY)
    if rc: return 1
    # Regex to match only the children.
    titleRegex = page.BLANK_TITLE_TEMPLATE.substitute(pageId=r"\d+")
    pages = cli.getPageList(title=page.BLANK_PARENT, descendants=True, titleRegex=titleRegex)
    print >>sys.stderr, len(pages), "pages"
    for p in pages:
        print page.PAGE_URL_TEMPLATE.substitute(cid=p["Id"])
    return 0

if __name__ == "__main__":
    try:
        sys.exit(main())
    except cli.AuthFailure, err:
        sys.exit(err.rc)

