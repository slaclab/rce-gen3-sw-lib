#!/bin/env python

import getpass, sys, re, argparse, subprocess
from HTMLParser import HTMLParser
from string import Template

import cli, page, fields

def add_serial(cid, serial="", force=False, dryrun=False):
    
    p = page.pageFromId(cid)

    print 80*"*"
    print cli.getPageTitle(cid)
    print 80*"*"
    print p
    print 80*"*"

    if serial:
        try:
            f = fields.SerialField(serial)
        except ValueError:
            return False
    else:
        try:
            f = fields.buildSerialField()
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

def getSerialFromCOB(address=""):
    
    ad_cmd_str = Template("ipmitool -I lan -H $shelf -t 0x20 -b 0 -A NONE raw 0x2c 0x01 0x00 0x00 0x03 $slot 0x00")
    id_cmd_str = Template("ipmitool -I lan -H $shelf -t $ipmb_addr -b 0 -A NONE raw 0x34 0x05 0x$bay")

    if not address:
        while True:
            address=raw_input("Enter <shelf manager>/<slot>/<bay> address to get ESN from: ").strip()
            try:
                (shelf,slot,bay) = address.split('/')
                break
            except ValueError:
                print "-ERROR- Invalid address"
                continue
    else:
        try:
            (shelf,slot,bay) = address.split('/')
        except ValueError:
            print "-ERROR- Invalid address"
            return None
        
        
    cmd = ad_cmd_str.substitute(shelf=shelf, slot=slot)
    try:
        out = subprocess.check_output(cmd, shell=True)
        ipmb_addr = "0x"+out.split()[2]
    except Exception, e:
        print "Couldn't get IPMB address from %s"%address
        print e
        return None
            
    cmd = id_cmd_str.substitute(shelf=shelf, ipmb_addr=ipmb_addr, bay=bay)
    try:
        out = subprocess.check_output(cmd, shell=True).split()
        out = "".join(out)
    except Exception, e:
        print "Couldn't get ESN from %s"%address
        print e
        return None

    return out


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

        if args.cob:
            if args.cob == "ASK":
                esn = getSerialFromCOB()
            else:
                esn = getSerialFromCOB(args.cob)
            if esn == None:
                return False
        elif args.esn:
            esn = args.esn
        else:
            esn = None

        return add_serial(cid=cid, 
                          serial=esn, 
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
This program is used to update/add Electronic Serial Number fields 
to the Hardeware Database. The ESN format will be checked to be 
consistent with a serial number from a DS28CM00.
"""
    epilog = """
Parameters that are not supplied on the command line will be prompted for.

The ESN is a 64 bit number in hex representation. 

The KEYWORD for searching for a page can be the pageId, URL, Short URL, 
AssetTag, Page Title, or other identifying content. It may need to be quoted. 

If you wish to fill the ESN with the value read from a COB Bay, specify the 
location with the Shelf Manager name/IP, Slot Number, and Bay Number. 
"""

    parser = argparse.ArgumentParser(description=desc,
                                     epilog=epilog,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)

    esn_group = parser.add_mutually_exclusive_group()
    esn_group.add_argument("-s", "--esn", help="The serial number to add")
    esn_group.add_argument("-c", "--cob", nargs='?', const="ASK",
                           help="COB bay address to retrieve ESN from",
                           metavar="<shelf-manager>/<slot>/<bay>")

    parser.add_argument("-p", "--page",
                        help="Search term for HWDB page", 
                        metavar="KEYWORD")
    parser.add_argument("-f", "--force", help="Allow overwriting of existing values",
                        action="store_true")
    parser.add_argument("-d", "--dryrun", action="store_true",
                        help="Do not save new page to HWDB")

    args = parser.parse_args()

    sys.exit(main(args))
    
