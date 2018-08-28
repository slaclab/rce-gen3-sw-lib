#!/bin/env python

import getpass, sys, re, argparse, subprocess
from HTMLParser import HTMLParser
from string import Template

import cli, page, fields
from update_field import add_field

def main(args):
    try:
        while True:
            try:
                pwd = getpass.getpass("Confluence password for %s: "\
                                          % getpass.getuser())
                rc = cli.login(getpass.getuser(), pwd, page.SPACE_KEY)
                break
            except cli.AuthFailure:
                pass

        if args.page:
            cid = page.extractPageId(args.page)
            if not cid:
                return False
        else:
            while True:
                    cid = page.askForPageId()
                    break

        return add_field(cid=cid, 
                         label="SLAC ID",
                         value=args.id,
                         force=args.force, 
                         dryrun=args.dryrun)

    except fields.AbortPage:
        print
        print "-ERROR- Page aborted."
    except KeyboardInterrupt:
        print
    return False


if __name__ == "__main__":

    desc = """ 
This program is used to update/add the SLAC ID field in the 
Hardeware Database. 
"""
    epilog = """
Parameters that are not supplied on the command line will be prompted for.

The ID is the new SLAC ID.

The KEYWORD for searching for a page can be the pageId, URL, Short URL, 
AssetTag, Page Title, or other identifying content. It may need to be quoted. 
"""

    parser = argparse.ArgumentParser(description=desc,
                                     epilog=epilog,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)

    parser.add_argument("-i", "--id",
                        help="The SLAC ID to add")

    parser.add_argument("-p", "--page",
                        help="Search term for HWDB page", 
                        metavar="KEYWORD")
    parser.add_argument("-f", "--force", help="Allow overwriting of existing values",
                        action="store_true")
    parser.add_argument("-d", "--dryrun", action="store_true",
                        help="Do not save new page to HWDB")

    args = parser.parse_args()

    sys.exit(main(args))
    
