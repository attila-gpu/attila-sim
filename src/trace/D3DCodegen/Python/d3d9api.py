import xml.sax
import xml.sax.handler
import pprint

class Parameter:
    def __init__(self):
        self.name = ""
        self.type = ""

class Function:
    def __init__(self):
        self.name = ""
        self.returns = ""
        self.parameters = []

class Method:
    def __init__(self):
        self.name = ""
        self.returns = ""
        self.parameters = []

class Interface:
    def __init__(self):
        self.name = ""
        self.base = ""
        self.methods = []

class StructMember:
    def __init__(self):
        self.name = ""
        self.type = ""

class Struct:
    def __init__(self):
        self.members = list()

class Enum:
    def __init__(self):
        self.members = list()

class D3D9API:
    def __init__(self):
        self.structs = list()
        self.enums = list()
        self.interfaces = list()
        self.functions = list()

class D3D9APIHandler(xml.sax.handler.ContentHandler):
    def __init__(self):
        self.D3D9API = D3D9API()
        self.path = "";
        
    def startElement(self, name, attributes):
        if self.path == "":
            self.path += "::" + name
        elif self.path == "::d3d9api":
            self.path += "::" + name
        elif (self.path == "::d3d9api::enums") & (name == "enum"):
            self.path += "::" + name
            self.enum = Enum()
            self.enum.name = attributes.getValue("name")
        elif (self.path == "::d3d9api::enums::enum") & (name == "member"):
            self.enum.members.append(attributes.getValue("name"))
        elif (self.path == "::d3d9api::structs") & (name == "struct"):
            self.path += "::" + name
            self.struct = Struct()
            self.struct.name = attributes.getValue("name")
        elif (self.path == "::d3d9api::structs::struct") & (name == "member"):
            m = StructMember()
            m.name = attributes.getValue("name");
            m.type = attributes.getValue("type");
            self.struct.members.append(m)
        elif (self.path == "::d3d9api::functions") & (name == "function"):
            self.path += "::" + name
            self.function = Function()
            self.function.name = attributes.getValue("name")
        elif (self.path == "::d3d9api::functions::function") & (name == "return"):
            self.function.returns = attributes.getValue("type")
        elif (self.path == "::d3d9api::functions::function") & (name == "parameters"):
            self.path += "::" + name
        elif (self.path == "::d3d9api::functions::function::parameters") & (name == "parameter"):
            p = Parameter()
            p.name = attributes.getValue("name")
            p.type = attributes.getValue("type")
            self.function.parameters.append(p)
        elif (self.path == "::d3d9api::interfaces") & (name == "interface"):
            self.path += "::" + name
            self.interface = Interface()
            self.interface.name = attributes.getValue("name")
            self.interface.base = attributes.getValue("base")
        elif (self.path == "::d3d9api::interfaces::interface") & (name == "method"):
            self.path += "::" + name
            self.method = Method()
            self.method.name = attributes.getValue("name")
        elif (self.path == "::d3d9api::interfaces::interface::method") & (name == "return"):
            self.method.returns = attributes.getValue("type")
        elif (self.path == "::d3d9api::interfaces::interface::method") & (name == "parameters"):
            self.path += "::" + name
        elif (self.path == "::d3d9api::interfaces::interface::method::parameters") & (name == "parameter"):
            p = Parameter()
            p.name = attributes.getValue("name")
            p.type = attributes.getValue("type")
            self.method.parameters.append(p)
      
    def endElement(self, name):
        if (self.path == "::d3d9api::structs") & (name == "structs"):
            self.path = "::d3d9api"
        elif (self.path == "::d3d9api::enums") & (name == "enums"):
            self.path = "::d3d9api"
        elif (self.path == "::d3d9api::enums::enum") & (name == "enum"):
            self.path = "::d3d9api::enums"            
            self.D3D9API.enums.append(self.enum)
        elif (self.path == "::d3d9api::structs") & (name == "structs"):
            self.path = "::d3d9api"
        elif (self.path == "::d3d9api::structs::struct") & (name == "struct"):
            self.D3D9API.structs.append(self.struct)
            self.path = "::d3d9api::structs"
        elif (self.path == "::d3d9api::functions") & (name == "functions"):
            self.path = "::d3d9api"
        elif (self.path == "::d3d9api::functions::function") & (name == "function"):
            self.path = "::d3d9api::functions"
            self.D3D9API.functions.append(self.function)
        elif (self.path == "::d3d9api::functions::function::parameters") & (name == "parameters"):
            self.path = "::d3d9api::functions::function"
        elif (self.path == "::d3d9api::interfaces") & (name == "interfaces"):
            self.path = "::d3d9api"
        elif (self.path == "::d3d9api::interfaces::interface") & (name == "interface"):
            self.path = "::d3d9api::interfaces"
            self.D3D9API.interfaces.append(self.interface)
        elif (self.path == "::d3d9api::interfaces::interface::method") & (name == "method"):
            self.path = "::d3d9api::interfaces::interface"
            self.interface.methods.append(self.method)
        elif (self.path == "::d3d9api::interfaces::interface::method::parameters") & (name == "parameters"):
            self.path = "::d3d9api::interfaces::interface::method"


def getD3D9API(xmlfile):
    parser = xml.sax.make_parser()
    handler = D3D9APIHandler()
    parser.setContentHandler(handler)
    parser.parse("d3d9api.xml")
    return handler.D3D9API
