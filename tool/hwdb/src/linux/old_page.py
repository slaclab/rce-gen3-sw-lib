import re
from HTMLParser import HTMLParser

## @brief Class to extract fileds from a page.
#
# A field is an <h6> element of the form <text>: <value>
# where the value can be more text or a reference to a page by title.
class OldFieldParser(HTMLParser):
    def __init__(self):
        HTMLParser.__init__(self)
        self.text = list()
        self.inField = False
        self.fields = list()
    def handle_starttag(self, tag, attrs):
        if tag == "h6":
            self.inField = True
        elif tag == "ac:rich-text-body":
            self.inField = True
        elif tag == "ri:page" and self.inField:
            attrs = dict(attrs)
            if "ri:content-title" in attrs:
                self.text.append(attrs["ri:content-title"])
    def handle_data(self, data):
        if self.inField:
            # We keep all the data in an element since items such as
            # a non-breaking space are considered non-data items, i.e.,
            # "foo&nbsp;bar" contains two data items: "foo" and "bar".
            self.text.append(data)
    def handle_endtag(self, tag):
        if tag == "h6" or tag == "ac:rich-text-body":
            self.inField = False
            self.fields.append(" ".join(self.text))
            self.text = list()
    # A field should have two tokens separated by
    # either by a colon, whitespace or colon and whitespace.
    # The first token is the field name and the second is
    # the value.
    fieldRe = re.compile(r"\s*([^:\s]+):?\s*(.*)")

    ## @brief  Return a dictionary of fields keyed by
    # lower-cased field names. The values are
    # not altered.
    def getDict(self):
        d = dict()
        for f in self.fields:
            mo = self.fieldRe.match(f)
            if mo:
                d[mo.group(1).lower()] = mo.group(2)
            else:
                raise RuntimeError
        return d

## @brief Extract the fields from page source. Returns a dictionary,
# possibly empty, keyed by field name.
def extractOldFields(pageSource):
    fp = OldFieldParser()
    fp.feed(pageSource)
    return fp.getDict()
