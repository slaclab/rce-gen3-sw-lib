from string import Template 
import re
import itertools

import page, hwpages, check

class TypeField(page.Field):
    def __init__(self, devtype):
        super(TypeField, self).__init__(key="Type", value="", 
                                        link=devtype, space=hwpages.HW_PAGE_SPACE)
    def isValid(self):
        valid_types = list(itertools.chain\
                               .from_iterable(hwpages.classToType.values()))
        return self.getLink() in valid_types
        
class SerialField(page.Field):
    def __init__(self, serial):
        super(SerialField, self).__init__(key="Serial", value=serial)
        
    def isValid(self):
        crc = check.serialCrc(self.getValue())
        if crc is None:
            print "-ERROR- Not a 16-digit hex number."
            return False
        elif crc[0] != crc[1]:
            print "-ERROR- Invalid CRC."
            print "        It's", hex(crc[0]), "in the number but I calculate", hex(crc[1])
            return False
        else:
            return True        

class AssetTagField(page.Field):
    def __init__(self, tag):
        super(AssetTagField, self).__init__(key="AssetTag", value=tag)
    
    def isValid(self):
        if self.getValue() == "":
            print "AssetTag must not be empty string"
            return False
        return True

class MACField(page.Field):
    def __init__(self, idx, mac):
        super(MACField, self).__init__(key="MAC%i"%idx, value=mac)
        
    def isValid(self):
        _macRe = re.compile(5*r"[\dA-Fa-f]{2,2}:" + r"[\dA-Fa-f]{2,2}$")    
        if _macRe.match(self.getValue()) is None:
            print "Invalid MAC address"
            return False
        return True


def buildTypeField():
    # Get the Device Class and Type
    devtype = None
    devclass = None
    while True:
        url = raw_input("Enter Board Type URL (<enter> to choose from list): ").strip()
        if url == ".":
            raise AbortPage
        if not url:
            break

        devtype  = page.extractPageTitle(url)
        devclass = hwpages.typeToClass.get(devtype, None)
        if not (devclass and devtype):
            print "-ERROR- Could not extract board type from URL %s"%url
        else:
            break

    # If entry by URL doesn't work, fall back to hand entry
    if not (devclass and devtype):
        # Get the device class.
        devclass = choose("Device class", hwpages.CLASSES)
        # Select the type fields based on device class.
        devtype  = choose("Type/HW revision", hwpages.classToType[devclass])

    # Set the fields appropriately
    return TypeField(devtype)

def buildField(label="", value="", link="", space=""):
    while not label:
        label = raw_input("Field Label: ").strip()
        if label == ".":
            raise AbortPage
        if label == "":
            print "Field must have a label!"

    while True:
        while not (value or link):
            value = raw_input("%s Value (<enter> for none): "%label).strip()
            if value == ".":
                raise AbortPage
            if value != "":
                break

            link = raw_input("%s Link Value: "%label).strip()
            if link == ".":
                raise AbortPage
            if link == "":
                print "Field must have a Value or a Link"
                value = None
                link  = None
            else:
                break

        if link:
            space = raw_input("Link Space (<enter> for none): ").strip()
            if space == ".":
                raise AbortPage
        else:
            space = ""
        
        try:
            return page.Field(label, value, link, space)
        except ValueError:
            print "Invalid Field, try again"
            pass
    

def buildSerialField():
   
    while True:
        ans = raw_input("Serial number: ").strip()
        if ans == ".":
            raise AbortPage
        if ans == "":
            return None
        try:
            return SerialField(ans)
        except ValueError:
            pass

def buildAssetTagField():
    while True:
        ans = raw_input("Enter an Asset Tag: ").strip()
        if ans == ".":
            raise AbortPage
        if ans == "":
            return None
        try:
            return AssetTagField(ans)
        except ValueError:
            pass

def buildMACFields(idx):
    mac_fields = []
    print "<enter> to end MACs, . to abort this page"
    while True:
        mac = raw_input("Enter MAC%i: "%idx).strip()
        if mac == ".":
            raise AbortPage
        if mac == "":
            return mac_fields
        try:
            mac_fields.append(MACField(idx, mac))
            idx += 1
        except ValueError:
            pass

def choose(description, choices):
    valid = xrange(len(choices))
    prompt = "Choose a %s: "%description
    while True:
        print "="*60
        print "%s:"%description
        for i, c in enumerate(choices):
            print "  ", i, choices[i]
        print "="*60
        ans = raw_input(prompt).strip()
        if ans == ".":
            raise AbortPage
        try:
            chosen = int(ans)
        except:
            chosen = -1
        if chosen not in valid:
            print "-ERROR- That's not a valid choice."
        else:
            return choices[chosen]

class AbortPage:
    pass

