#!/usr/bin/env python

"""
mxp, 20231128
Generate c code file from json object

now support type:
idl            c             json
int      ->    int    ->     Number
double   ->    double  ->    Number
string   ->    char*    ->   String : memory will allocate when assigned
char name[num]   ->    char[num]   ->   String : using preallocate memory no need allocate & free
string:size
string[num]  ->   char* [num] ->   Array
string[] ->     char**   -> Array : unknown using list link
"""
import os
import re
import sys
import string
import collections
import argparse
import codecs;

known_attribute_words = ["attribute", "readonly"]
known_type_words = ["int", "double", "bool", "string", "JSON", "Object", "Array"]

class J2CObject:
  def __init__(self):
    self.module=""
    self.construct_p=[]
    self.attributes = []
    #self.methods = [] #collections.OrderedDict()

class J2CAttribute:
  def __init__(self):
    self.type = ""
    self.attribute = ""
    # array size when type is array
    self.size = 0

def parserAttribute(obj, statement):
  attr = J2CAttribute()
  statement = statement[:statement.find(';')]
  statement = statement.strip();
  arr = statement.split(' ')
  for i in arr:
    if i == "":
      continue;
    elif i in known_type_words:
      attr.type = i
    else:
        attr.attribute = i
  obj.attributes.append(attr)




def parserIdl(idl, obj):
  """ main work for idl read"""

  with codecs.open(idl, "rb", "UTF-8") as f:
    need_process_declare = True
    for line in f:
      line = line.strip()
      if line == "" or line.startswith("#") or line.startswith("//"):
        continue
      if need_process_declare:
        idx = line.find('{')
        line = line[:idx]
        obj.module = line.split(' ')[1].strip()

        need_process_declare = False
        continue;

      """check attribute and functions"""
      arr = line.split(" ")
      if arr[0] in known_type_words:
        parserAttribute(obj, line);
        continue;

def j2cTypeFromJson(attr):
  if attr.type == "string":
    return "J2C_STRING"
  elif attr.type == "int":
    return "J2C_INT"
  elif attr.type == "double":
    return "J2C_DOUBLE"
  if attr.type == "Array":
    return "J2C_ARRAY"

  return "unknown"

def cTypeFromJson(attr):
  if attr.type == "string":
    return "char*"
  elif attr.type == "int":
    return "int"
  elif attr.type == "double":
    return "double"
  elif attr.type == "Object":
    return "struct j2cobject*"
  elif attr.type == "Array":
    return "struct j2cobject*"

  return "unknown"
def generateHeader(obj):
  out = open("j2cobject_"+obj.module.lower()+".h", "wb")

  out.write("/*file is auto created , do not modify it!!!*/\n")
  out.write('''/*
* Copyright (C) 2023 Inspur Group Co., Ltd. Unpublished
*
* Inspur Group Co., Ltd.
* Proprietary & Confidential
*
* This source code and the algorithms implemented therein constitute
* confidential information and may comprise trade secrets of Inspur
* or its associates, and any use thereof is subject to the terms and
* conditions of the Non-Disclosure Agreement pursuant to which this
* source code was originally received.
*/
''')
  out.write("\n")
  out.write("#ifndef _J2COBJECT_"+obj.module.upper()+"_H_\n")
  out.write("#define _J2COBJECT_"+obj.module.upper()+"_H_\n")
  out.write("\n")
  
  out.write("#include \"stddef.h\"\n")
  out.write("#include \"stdlib.h\"\n")
  out.write("#include \"j2cobject.h\"\n")
  out.write("\n\n")

  """declare object struct"""
  out.write("struct j2cobject_" + obj.module.lower() + " {\n")

  out.write("  J2COBJECT_DECLARE_OBJECT;\n")
  for a in obj.attributes:
    out.write("  " + cTypeFromJson(a) + " " + a.attribute + ";\n")

  out.write("};\n")
  out.write("\n")

  """declare object prototype"""
  out.write("static struct j2cobject_prototype j2cobject_prototype_" + obj.module.lower() + "[] = {\n")
  for a in obj.attributes:
    out.write("  {\n")
    out.write("    .name= \"" + a.attribute + "\",\n")
    out.write("    .type= " + j2cTypeFromJson(a) + ",\n")
    out.write("    .offset= offsetof(struct j2cobject_" + obj.module.lower() + ", " + a.attribute + "),\n")
    out.write("    .offset_len= {}\n".format(a.size))
    out.write("  },\n")

  out.write("  {0}\n")
  out.write("};\n")
  out.write("\n")
   
  out.write("static inline struct j2cobject_" + obj.module.lower() + "* j2cobject_" + obj.module.lower() + "_allocate() {\n")
  #out.write("  struct j2cobject *obj = (struct j2cobject*)calloc(1, sizeof(" + "struct j2cobject_" + obj.module.lower() + "));\n");
  out.write("  struct j2cobject *obj = j2cobject_create(J2C_OBJECT, sizeof(" + "struct j2cobject_" + obj.module.lower() + "));\n")
  out.write("  if (!obj) return NULL;\n")
  out.write("  obj->name = \"" + obj.module +"\";\n")
  out.write("  obj->prototype = j2cobject_prototype_" + obj.module.lower() + ";\n")
  out.write("  return (struct j2cobject_" + obj.module.lower() +"*)obj;\n")
  out.write("}\n")
  out.write("static inline void j2cobject_" + obj.module.lower() + "_deallocate(struct j2cobject_" + obj.module.lower() +"* obj) {\n")
  out.write("  if (!obj) return;\n")
  # loop attribute find all string release it
  #for a in obj.attributes:
  #  if a.type == "string":
  #    out.write("  if (obj->" + a.attribute + ") free(obj->" + a.attribute + ");\n")
  #out.write("  free(obj);\n")
  out.write("   j2cobject_free(J2COBJECT(obj));\n")
  out.write("}\n")
  out.write("#endif //_J2COBJECT_"+obj.module.upper()+"_H_")
  out.write("\n")
  out.close()

def main(argv):
  parser = argparse.ArgumentParser(description='Parsing IDL for J2C')
  #parser.add_argument("--local", action='store_true', help="the Local Javascript Name")
  parser.add_argument("idl", action="store")

  args = parser.parse_args();

  obj = J2CObject()
  parserIdl(args.idl, obj)
  generateHeader(obj)

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
