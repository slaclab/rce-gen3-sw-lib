#!/bin/env python
#-*-Mode: Python;-*-
## @file
# @brief Create blank pages and print their page-id URLs.
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
# \$Date: 2015-05-08 23:29:48 -0700 (Fri, 08 May 2015) $ by \$Author: jgt $.
#
# @par Revision number:
# \$Revision: 4070 $
#
# @par Location in repository:
# \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/hwdb/src/linux/makeblanks.py $
#
# @par Credits:
# SLAC

# Python library.
import datetime, getpass, os, sys

# HWDB software.
import cli, page


def main():
    count = getint("Number of new pages: ")
    pwd = getpass.getpass("Confluence password: ")
    if cli.login(getpass.getuser(), pwd, page.SPACE_KEY):
        print "Confluence login failed."
        return 1

    stamp = datetime.datetime.now().strftime("%Y_%m_%d_%H%M%S")
    fname = "hwdb_blanks_"+stamp+".csv"
    outfile = open(fname , mode="w")
    print "Writing URLs to file: %s"%fname
    outfile.write("Long URL, Short URL\n")

    blank_page = page.Page()

    for i in xrange(count):
        # Create the page using a title based on process ID and current date+time,
        # to ensure uniqueness. No page ID is available until after creation.
        stamp = str(os.getpid()) + datetime.datetime.utcnow().isoformat()
        title = page.BLANK_TITLE_TEMPLATE.substitute(pageId=stamp)
        cid = cli.addPage(title,
                          parentTitle=page.BLANK_PARENT,
                          content=blank_page.getMarkup())
        if not cid:
            return 1
        # Rename the page to a title with the true page ID.
        if not cli.renamePage(page.BLANK_TITLE_TEMPLATE.substitute(pageId=cid), cid=cid):
            return 1

        long_url = page.PAGE_URL_TEMPLATE.substitute(cid=cid)
        short_url = page.extractPageShortURL(long_url)
        print "%s, %s"%(long_url, short_url)
        outfile.write("%s, %s\n"%(long_url, short_url))
    return 0
        
def getint(prompt):
    val = -1
    while val <= 0:
        answer = raw_input(prompt)
        try:
            val = int(answer)
            if val <= 0:
                print "Type a positive integer."
        except:
            print "Type a positive integer."
            val = -1
    return val


if __name__ == "__main__":
    try:
        sys.exit(main())
    except cli.AuthFailure, err:
        sys.exit(err.rc)

