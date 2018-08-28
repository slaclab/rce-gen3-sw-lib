#-*-Mode: Python;-*-
## @file
# @brief Provide basic Confluence operations using its Command Line Interface.
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
# \$Date: 2016-01-19 14:58:51 -0800 (Tue, 19 Jan 2016) $ by \$Author: jgt $.
#
# @par Revision number:
# \$Revision: 4647 $
#
# @par Location in repository:
# \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/hwdb/src/linux/cli.py $
#
# @par Credits:
# SLAC

# Standard Python library.
import csv
import re
import subprocess
import sys

# The Confluence password for the user.
_password = None

# The latest login token for the user.
_token = None

# The user name.
_user = None

# The Confluence space to operate upon.
_space = None

# The path to the Confluence CLI program.
#_program = "/afs/slac.stanford.edu/g/scs/entapps/confluence-cli-3.4.0/confluence-prod"
_program = "/afs/slac.stanford.edu/g/scs/entapps/confluence-cli-3.9.0/confluence-prod"

# Return code for authentication failure. Could be due to a bad
# username, a bad password, or invalid/expired token.
_AUTH_FAILURE = 253

## @brief Exception to be raised in case of an authentication
# failure. Contains the return code in the "rc" attribute.
class AuthFailure:
    def __init__(self, rc):
        self.rc = rc

# Run a general command as a subprocess, adding login
# information. Print stdout and stderr if the command fails. In any
# case return the tuple (return code, stdout, stderr).
# args - A list of execve arguments for Popen.
def _run(args, force_password=False):
    args = [_program] + list(args)
    args += ["--user", _user, "--space", _space]
    if _token and not force_password:
        # Password is required but can be empty when we pass a login token.
        args += ["--password", "", "--login", _token]
    elif _password:
        args += ["--password", _password]
    else:
        print >>sys.stderr, "Attempted an operation before logging in."
        raise AuthFailure(1)
    rc, stdout, stderr = _run2(args)
    if rc:
        print >>sys.stderr, "Error code", rc
        print >>sys.stderr, stderr
        if rc == _AUTH_FAILURE: raise AuthFailure(rc)
    return rc, stdout, stderr

def _run2(args):
    sub = subprocess.Popen(args,
                           shell=False,
                           universal_newlines=True,
                           stdout=subprocess.PIPE,
                           stderr=subprocess.PIPE)
    stdout, stderr = sub.communicate()
    rc = sub.returncode
    return rc, stdout, stderr

## @brief Set user name, password and Confluence space key. Log in and
## obtain a token for use
# by subsequent operations.
# @return The return code from the login operation.
def login(user, password, space):
    global _user, _password, _space, _token
    _user = user
    _password = None
    _password = password
    _space = space
    _token = None
    rc, stdout, stderr = _run(["--action", "login"])
    if not rc:
        _token = stdout.strip()
        #_password = None
    return rc

## @brief Return the current value of the login token, a string.
def getToken():
    return _token

## @brief Set the token, user name and Confluence space key for use by
## future operations.
def setLogin(token, user, space):
    global _token, _user, _space
    _token = token
    _user = user
    _space = space

## @brief Add a page to Confluence and return its ID (or a null string if failure).
def addPage(title, parentTitle, content):
    args = ["--action", "addPage", "--title", title, "--parent", parentTitle, "--content", content,
            "--noConvert"]
    rc, stdout, stderr = _run(args)
    if rc:
        return ""
    # The page ID is the last word of the response.
    return stdout.split()[-1]

## @brief Get a list of descriptions of pages matching certain criteria.
# @param[in] title Page title. Provide either this or cid.
# @param[in] cid Confluence ID of a page. Provide either this or title.
# @param[in] ancestors If true then include also the ancestors of the page identified by title/cid.
# @param[in] descendants If true then include also the descendants of the page identified by title/cid.
# @param[in] titleRegex If not None then keep only those pages that match this Perl regex string.
# The regex must match the entire title.
#
# Returns a possibly empty list of dictionaries, where each dictionary
# describes a page based on CSV-format output from the CLI. Known valid keys:
# - Id
# - Title
# - URL
# - parentId
def getPageList(title=None, cid=None, ancestors=False, descendants=False, titleRegex=None):
    args = ["--action", "getPageList", "--outputFormat", "2"]
    if title:
        args += ["--title", title]
    else:
        # --id isn't accepted so we have to use the page title.
        title = getPageTitle(cid)
        if not title:
            return []
        args += ["--title", title]
    if ancestors:
        args.append("--ancestors")
    if descendants:
        args.append("--descendents")
    if titleRegex:
        args += ["--regex", titleRegex]
    rc, stdout, stderr = _run(args)
    if rc:
        return list()
    else:
        return _parseCsv(stdout)

## @brief Some operations will take a title but not a page ID.
# Use getPageTitle() with an ID to find the title. Returns None
# if the page doesn't exist.
def getPageTitle(cid):
    args = ["--action", "getPage", "--id", cid]
    rc, stdout, stderr = _run(args)
    if rc:
        return None
    mo = _pageTitleRe.search(stdout)
    return mo.group(1)
_pageTitleRe = re.compile(r"^Page title[^:]+: (.*)$", re.MULTILINE)

def _parseCsv(text):
    # Parse Excel-compatible CSV format preceded by a line
    # giving the number of pages found. The first line in CSV form
    # gives the names of the fields. We'll use that to construct
    # a csv.DictReader for the rest of the output.
    lines = text.splitlines()
    try:
        count = int(lines[0].split()[0])
    except ValueError:
        return []
    if count == 0:
        return list()
    del lines[0]
    reader = csv.DictReader(lines)
    return list(reader)

## @brief Get the XTML source for a page, which may be identified by
# its page ID or by its title. Returns None if failure.
def getPageSource(cid=None, title=None):
    args = ["--action", "getPageSource"]
    args += (["--id", cid] if cid else ["--title", title])
    rc, stdout, stderr = _run(args)
    return (None if rc else stdout)

## @brief Add a comment to a page which is identified by title. Returns True if
# the operation succeeded, else False.
def addComment(title, comment):
    args = ["--action", "addComment", "--title", title, "--comment", comment]
    rc, stdout, stderr = _run(args)
    return (False if rc else True)

def getComments(title):
    args = ["--action", "getComments", "--title", title]
    rc, stdout, stderr = _run(args)
    if rc:
        return None
    else:
        return stdout

def renderRequest(url):
    args = ["--action", "renderRequest", "--request", url]
    rc, stdout, stderr = _run(args, force_password=True)
    if rc:
        return None
    else:
        return stdout

## @brief Move a page to a different parent. Returns True for success, False for failure.
def movePage(parentTitle, cid=None,  title=None):
    args = ["--action", "movePage", "--parent", parentTitle]
    if title:
        args += ["--title", title]
    else:
        args += ["--id", cid]
    rc, stdout, stderr = _run(args)
    return (False if rc else True)

## @brief Rename a page, possibly moving it to a new parent (identified by title).
# Returns True if success else False.
def renamePage(newTitle, cid=None, title=None, newParent=None):
    args = ["--action", "renamePage", "--newTitle", newTitle]
    if title:
        args += ["--title", title]
    else:
        args += ["--id", cid]
    if newParent:
        args += ["--parent", newParent]
    rc, stdout, stderr = _run(args)
    return (rc == 0)

## @brief  Replace the content of an existing page, or make a new one, optionally adding a comment.
# Returns True if success else False.
#
# Content is in storage format.
# Note that any comment added by this operation is visible only
# in the page version history if one is using the Document layout
# for one's space.
def storePage(newContent, cid=None, title=None, comment=None):
    args = ["--action", "storePage", "--noConvert"]
    if title:
        args += ["--title", title]
    else:
        args += ["--id", cid]
    args += ["--content", newContent]
    if comment:
        args += ["--comment", comment]
    rc, stdout, stderr = _run(args)
    return (rc == 0)

## @brief Get a list of pages that satisfy a Confluence content search, using the
# same search terms that one uses in search fields on Confluence pages.
#
# The search results are parseable using _parseCsv(). The fields per
# line are Id, Type, Title, URL, and Excerpt. Type will always be "page"
# here. The excerpt is text surrounding the text that matched. It's just
# text without any XHTML or HTML tags. The whole space is searched.
# Failure returns an empty list.
def getContentList(searchTerms):
    args = ["--action", "getContentList", "--searchTypes", "page", "--search", searchTerms]
    rc, stdout, stderr = _run(args)
    if rc:
        return []
    return _parseCsv(stdout)

def test():
    import getpass, pprint
    user = getpass.getuser()
    password = getpass.getpass()
    login(user, password, "~" + user)
    pages = getPageList(title="000000000067 DPM")
    pprint.pprint(pages)
    addComment("000000000067 DPM", "This is a comment!")
    addComment("frumious bandersnatch", "Failed comment.")
    addPage("frumious bandersnatch", "000000000067 DPM", "Page content.")
    movePage("Database master", title="frumious bandersnatch")

def test2():
    import getpass
    user = getpass.getuser()
    password = getpass.getpass()
    login(user, password, "CCI")
    pages = getPageList(title="Unused HWDB pages", descendants=True, titleRegex=r"Unused HWDB page [0-9]+")
    print len(pages), "pages"
    for p in pages:
        print "https://confluence.slac.stanford.edu/pages/viewpage.action?pageId="+p["Id"]
        print p["Title"]
