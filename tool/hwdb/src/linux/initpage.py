#!/bin/env python

# Python library.
import datetime, getpass, re, sys, argparse
from string import Template

# HWDB code.
import check, cli, page, hwpages, sites, fields
                     
def init_page(cid, devtype="", tag="", status="", site="", dryrun=False):

    p = page.pageFromId(cid)
    title_base = cli.getPageTitle(cid)

    print 80*"*"
    print title_base
    print 80*"*"
    print p
    print 80*"*"

    if not p.isBlank():
        print "-ERROR- Can only run initpage on a blank page"
        return False

    if devtype:
        try:
            f = fields.TypeField(devtype)
        except ValueError:
            return False
    else:
        f = fields.buildTypeField()

    if not f or not p.addField(f, force=True):
        print "Couldn't add Type - Aborting Page Init"
        return False

    devclass = hwpages.typeToClass.get(f.getLink(), None)

    if tag:
        try:
            f = fields.AssetTagField(tag)
        except ValueError:
            return False
    else:
        f = fields.buildAssetTagField()

    tag = f.getValue()

    if not f or not p.addField(f, force=True):
        print "Couldn't add AssetTag - Aborting Page Init"
        return False

    if not status:
        status = raw_input("Board Status: ")

    p.setStatus(status)

    comment = "Born: " + datetime.datetime.utcnow().isoformat(" ") + " UTC"
    new_title = devclass + " " + tag + ' - ' + title_base

    if not site:
        site = fields.choose("New Location", sites.CONTAINER_PAGES)

    if not dryrun:
        if not cli.storePage(p.getMarkup(), cid=cid):
            print "Page content update failed!"
            return False

        if not cli.addComment(title_base, comment):
            print "Adding a comment failed!"
            return False

        if not cli.renamePage(newTitle=new_title, cid=cid, newParent=site):
            print "Moving the page failed!"
            return False
    else:
        print "Dry Run, not writing to HWDB"

    print "*"*80
    print "%s/%s"%(site, new_title)
    print "*"*80
    print p
    print "*"*80

    return True

def main(args):
    print """
Type control-D or control-C at a prompt to terminate.
While making a page type a period to abort the page.
"""
    while True:
        try:
            pwd = getpass.getpass("Confluence password for %s: "\
                                      % getpass.getuser())
            rc = cli.login(getpass.getuser(), pwd, page.SPACE_KEY)
            break
        except cli.AuthFailure:
            pass

    try:
        if args.page:
            cid = page.extractPageId(args.page)
            if not cid:
                return False
            return init_page(cid=cid, 
                             devtype=args.board, 
                             tag=args.tag,
                             status=args.status,
                             site=args.loc,
                             dryrun=args.dryrun)
        
        # If the page wasn't specified on the command line,
        # just keep asking for new pages.
        else:
            while True:
                cid = page.askForPageId()
                init_page(cid=cid, 
                          devtype=args.board, 
                          tag=args.tag,
                          status=args.status,
                          site=args.loc,
                          dryrun=args.dryrun)

    except fields.AbortPage:
        print
        print "-ERROR- Page aborted."
    except KeyboardInterrupt:
        print
        return False

if __name__ == "__main__":

    desc = """ 
This program will initialize a blank HWDB page with the Device Type,
Asset Tag, and initial status.
"""
    epilog = """
Parameters that are not supplied on the command line will be prompted for.

The Board TYPE must be one of the types found in hwpages.py which in turn
are the titles of the board design pages in AIRTRACK/PCB Designs.

The Asset Tag TAG is the Human-readable label placed on the board 
by the E-shop.

The Board STATUS is a string that describes the state of the board. It
will default to "Received"

The Board Location (or Site) is the new parent page. Once initialized,
the new page will be made a child of the Site page. The possible pages
are listed in sites.py. 

The KEYWORD for searching for a page can be the pageId, URL, Short URL, 
AssetTag, Page Title, or other identifying content. It may need to be quoted. 
"""

    parser = argparse.ArgumentParser(description=desc,
                                     epilog=epilog,
                                     formatter_class=argparse\
                                         .RawDescriptionHelpFormatter)

    parser.add_argument("-b", "--board", metavar="TYPE", 
                        help="Board type")
    parser.add_argument("-t", "--tag", 
                        help="Asset tag")
    parser.add_argument("-s", "--status", default="Received",
                        help="Board status")
    parser.add_argument("-l", "--loc", 
                        help="Board site")

    parser.add_argument("-p", "--page",
                        help="Search term for HWDB page", 
                        metavar="KEYWORD")

    parser.add_argument("-d", "--dryrun", action="store_true",
                        help="Do not save new page to HWDB")

    args = parser.parse_args()

    sys.exit(main(args))
