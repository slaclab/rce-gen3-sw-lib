#!/bin/env python

import getpass, sys, re, argparse, subprocess
from HTMLParser import HTMLParser
from string import Template

import cli, page, fields

def add_field(cid, 
              label="", value="",  link="", space="", 
              force=False, dryrun=False):
    
    p = page.pageFromId(cid)

    print 80*"*"
    print cli.getPageTitle(cid)
    print 80*"*"
    print p
    print 80*"*"

    if label and (value or link):
        try:
            f = fields.Field(label, value, link, space)
        except ValueError:
            return False
    else:
        try:
            f = fields.buildField(label, value, link, space)
        except fields.AbortPage:
            print "Page Aborted"
            return False

    if not f or not p.addField(f, force=force):
        print "Couldn't add Serial - Aborting Page Update"
        return False
    
    print 80*"*"
    print cli.getPageTitle(cid)
    print 80*"*"
    print p
    print 80*"*"

    if dryrun:
        print "Dry Run, not writing to HWDB"
        return True

    return cli.storePage(p.getMarkup(), cid=cid)

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
                         label=args.key,
                         value=args.value,
                         link=args.link,
                         space=args.space,
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
This program is used to update/add an arbitrary field to the 
Hardeware Database. 
"""
    epilog = """
Parameters that are not supplied on the command line will be prompted for.

The LABEL is used for the key of the field.

The VALUE is plain text that will be displayed right of the label.

The LINK and SPACE can be specified in place of the label if a link 
is desired. The SPACE will default to The Hardware Tracking Space.

The KEYWORD for searching for a page can be the pageId, URL, Short URL, 
AssetTag, Page Title, or other identifying content. It may need to be quoted. 
"""

    parser = argparse.ArgumentParser(description=desc,
                                     epilog=epilog,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)

    parser.add_argument("-k", "--key", metavar="LABEL",
                        help="The label to add")

    valink_group = parser.add_mutually_exclusive_group()
    valink_group.add_argument("-v", "--value",
                              help="The value to add")
    valink_group.add_argument("-l", "--link", 
                              help="Confluence page Title to link to")

    parser.add_argument("-s", "--space",
                        help="Confluence space to find link in. Defaults to %s"%page.SPACE_KEY, 
                        default=page.SPACE_KEY)

    parser.add_argument("-p", "--page",
                        help="Search term for HWDB page", 
                        metavar="KEYWORD")
    parser.add_argument("-f", "--force", help="Allow overwriting of existing values",
                        action="store_true")
    parser.add_argument("-d", "--dryrun", action="store_true",
                        help="Do not save new page to HWDB")

    args = parser.parse_args()

    sys.exit(main(args))
    
