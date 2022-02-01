#
# @file locking_generator.py
#
# @brief Generate C code from JSON document.
#
# Copyright (c) 2018-2022 Laird Connectivity
#
# SPDX-License-Identifier: Apache-2.0
#
import json
import jsonref
import collections
import os
import inflection
import sys
import math

JSON_INDENT = '  '

# Left Justified Field Widths
ID_WIDTH = 54
NAME_MACRO_WIDTH = 20
DEFINE_WIDTH = 20
TYPE_WIDTH = 24
COUNT_LIMIT_WIDTH = 12

BASE_FILE_PATH = "./custom/%PROJ%"
HEADER_FILE_PATH = "%BASE%/include/"
SOURCE_FILE_PATH = "%BASE%/source/"
TABLE_FILE_NAME = "locking_table"

def ToInt(b) -> str:
    return math.trunc(b)

def GetNumberField(d: dict, item: str):
    """ Handles items not being present (lazy get) """
    try:
        r = d[item]
        return r
    except:
        return 0.0

def GetBoolField(d: dict, item: str):
    try:
        r = d[item]
        return r
    except:
        return False

def GetStringField(d: dict, item: str):
    try:
        r = d[item]
        return r
    except:
        return ""

def GetDictionaryField(d: dict, item):
    r = {}
    try:
        r = d[item]
        return r
    except:
        return r

def PrintDuplicate(lst):
    for item, count in collections.Counter(lst).items():
        if count > 1:
            print(item)

class locks:
    def __init__(self, project: str, fname: str):

        # The following items are loaded from the configuration file
        self.parameterList = 0
        self.projectLocksCount = 0
        self.apiTotalLocks = 0
        self.MaxNameLength = 0
        self.project = project
        self.largestNumberSize = 0

        self.id = []
        self.apiId = []
        self.count = []
        self.limit = []
        self.name = []
        self.apiName = []
        self.type = []

        self.IncrementVersion(fname)
        self.LoadConfig(fname)

    def IncrementVersion(self, fname: str) -> None:
        """
        Increment version of the form x.y.z and write it back to the file.
        """
        with open(fname, 'r') as f:
            data = json.load(f)
            api_version = data['info']['version']
            major, minor, build = data['info']['version'].split('.')
            build = int(build) + 1
            new_version = f'{major}.{minor}.{build}'
            data['info']['version'] = new_version
            print(new_version)

        with open(fname, 'w') as f:
            json.dump(data, f, indent=JSON_INDENT)

    def LoadConfig(self, fname: str) -> None:
        with open(fname, 'r') as f:
            data = jsonref.load(f)
            self.parameterList = data['components']['contentDescriptors']['deviceParams']['x-device-locks']
            self.apiTotalLocks = len(self.parameterList)
            file_name = HEADER_FILE_PATH + TABLE_FILE_NAME
            self.inputHeaderFileName = file_name
            self.outputHeaderFileName = file_name + ""
            file_name = SOURCE_FILE_PATH + TABLE_FILE_NAME
            self.inputSourceFileName = file_name
            self.outputSourceFileName = file_name + ""

            # Extract the properties for each parameter
            for p in self.parameterList:
                self.apiName.append(p['name'])
                self.apiId.append(p['x-id'])
                if self.project in p['x-projects']:
                    # required fields
                    self.name.append(p['name'])
                    self.id.append(p['x-id'])
                    # required schema fields
                    a = p['schema']
                    self.type.append(a['type'])
                    # optional schema fields have a default value
                    self.count.append(ToInt(GetNumberField(a, 'count')))
                    self.limit.append(ToInt(GetNumberField(a, 'limit')))

            self.projectLocksCount = len(self.name)
            print(f"API Total Locks {self.apiTotalLocks}")
            print(
                f"Project {self.project} Locks {self.projectLocksCount}")
            print(f"Project {self.project} Maximum ID {max(self.id)}")
            self.PrintAvailableIds()
            pass

    def GetType(self, index: int) -> str:
        kind = self.type[index]
        s = "LOCKING_TYPE_"
        if kind == "mutex":
            s += "MUTEX"
        elif kind == "semaphore":
            s += "SEMAPHORE"
        else:
            s += "UNKNOWN"

        return s

    def GetLockMacro(self, index: int) -> str:
        """Get the c-macro for the lock"""
        name = self.name[index]
        s = "LOCK(" + name + ")"
        return s.ljust(NAME_MACRO_WIDTH)

    def CreateCountLimitString(self, index: int) -> str:
        """
        Create the count/limit portion of the lock table entry for semaphores
        """
        kind = self.type[index]
        i_min = self.count[index]
        i_max = self.limit[index]
        i_min = int(i_min)
        i_max = int(i_max)
        s_min = f".count = " + str(i_min)
        s_max = f".limit = " + str(i_max)

        return s_min.ljust(COUNT_LIMIT_WIDTH) + ", " + s_max.ljust(COUNT_LIMIT_WIDTH)

    def GetIndex(self, name: str) -> int:
        """ Get the index for a key/name """
        for i in range(self.projectLocksCount):
            if self.name[i] == name:
                return i
        return -1

    def CreateAttrTable(self) -> str:
        """
        Create the lock (property) table from the dictionary of lists
        created from the Excel spreadsheet and gperf
        """
        lockTable = []
        for i in range(self.projectLocksCount):
            result = f"\t[{i:<3}] = " \
                + "{ " + f"{self.id[i]:<3}, " \
                + f"{self.GetLockMacro(i)}, {self.GetType(i).ljust(TYPE_WIDTH)}, " \
                + f"{self.CreateCountLimitString(i)}" \
                + " }," \
                + "\n"
            lockTable.append(result)

        lockTable.append("\n")

        string = ''.join(lockTable)
        return string[:string.rfind(',')] + '\n'

    def CreateInit(self) -> str:
        """
        Create the lock initialisation code from the dictionary of lists
        """
        lockTable = []
        for i in range(self.projectLocksCount):
            kind = self.type[i]
            name = self.name[i]
            if kind == "mutex":
                result = f"\tk_mutex_init(&{name});\n"
            elif kind == "semaphore":
                result = f"\tk_sem_init(&{name}, {int(self.count[i])}, {int(self.limit[i])});\n"
            lockTable.append(result)

        string = ''.join(lockTable)
        return string

    def CreateReset(self) -> str:
        """
        Create the lock reset code from the dictionary of lists
        """
        lockTable = []
        for i in range(self.projectLocksCount):
            kind = self.type[i]
            name = self.name[i]
            if kind == "semaphore":
                result = f"\tk_sem_reset(&{name});\n"
                lockTable.append(result)

        string = ''.join(lockTable)
        return string

    def CheckForDuplicates(self) -> bool:
        """
        Check for duplicate parameter IDs or names.
        """
        if len(set(self.id)) != len(self.id):
            print("Duplicate lock ID in Project")
            PrintDuplicate(self.apiName)
            return False

        if len(set(self.apiId)) != len(self.apiId):
            print("Duplicate lock ID in API")
            PrintDuplicate(self.apiName)
            return False

        if len(set(self.name)) != len(self.name):
            print("Duplicate lock Name")
            PrintDuplicate(self.apiName)
            return False

        if len(set(self.apiName)) != len(self.apiName):
            print("Duplicate lock Name in API")
            PrintDuplicate(self.apiName)
            return False

        return True

    def CheckForValidOptions(self) -> bool:
        """
        Check for valid options
        """
        for i in range(self.projectLocksCount):
            kind = self.type[i]
            if kind == "semaphore":
                i_count = self.count[i]
                i_limit = self.limit[i]

                if i_limit < 1:
                    print(f"Semaphore limit must be >= 1:" +
                          f" {self.name[i]} with limit {i_limit}")
                    return False
                elif i_count > i_limit:
                    print(f"Semaphore count must be less than or equal to limit:" +
                          f" {self.name[i]} with count {i_count} and limit {i_limit}")
                    return False

        return True

    def UpdateFiles(self) -> None:
        """
        Update the lock c/h files.
        """
        if self.CheckForDuplicates() == False:
            return
        if self.CheckForValidOptions() == False:
            return

        self.CreateSourceFile(
            self.CreateInsertionList(SOURCE_FILE_PATH + TABLE_FILE_NAME + ".c"))
        self._CreateLockHeaderFile(
            self.CreateInsertionList(HEADER_FILE_PATH + TABLE_FILE_NAME + ".h"))

    def CreateInsertionList(self, name: str) -> list:
        """
        Read in the c/h file and create a list of strings that
        is ready for the lock information to be inserted
        """
        print("Reading " + name)
        lst = []
        with open(name, 'r') as fin:
            copying = True
            for line in fin:
                if "pystart" in line:
                    lst.append(line)
                    copying = False
                elif "pyend" in line:
                    lst.append(line)
                    copying = True
                elif copying:
                    lst.append(line)

        return lst

    def CreateStruct(self, remove_last_comma: bool) -> str:
        """
        Creates the structures and default values for locks.
        """
        struct = []
        for i in range(self.projectLocksCount):
            name = self.name[i]
            # string is required in test tool, c requires char type
            if self.type[i] == "mutex":
                kind = "struct k_mutex"
            elif self.type[i] == "semaphore":
                kind = "struct k_sem"

            # Use tabs because we use tabs with Zephyr/clang-format.
            result = f"static {kind} {name};" + "\n"
            struct.append(result)

        string = ''.join(struct)
        return string

    def CreateMap(self) -> str:
        """
        Create map of ids to table entries
        Invalid entries are NULL
        """
        s = ""
        for i in range(max(self.id) + 1):
            if i in self.id:
                idx = self.id.index(i)
                s += f"\t[{i:<3}] = &LOCKING_TABLE[{idx:<3}],\n"

        s = s[:s.rfind(',')] + '\n'

        return s

    def PrintAvailableIds(self):
        available = []
        for i in range(self.apiTotalLocks):
            if i not in self.apiId:
                available.append(i)

        print(f"Available API IDs\n {available}")

    def CreateSourceFile(self, lst: list) -> None:
        """Create the settings/lock/properties *.c file"""
        name = SOURCE_FILE_PATH + TABLE_FILE_NAME + ".c"
        print("Writing " + name)
        with open(name, 'w') as fout:
            for index, line in enumerate(lst):
                next_line = index + 1
                if "pystart - " in line:
                    if "locking table" in line:
                        lst.insert(next_line, self.CreateAttrTable())
                    elif "locking map" in line:
                        lst.insert(next_line, self.CreateMap())
                    elif "init" in line:
                        lst.insert(next_line, self.CreateInit())
                    elif "reset" in line:
                        lst.insert(next_line, self.CreateReset())
                    elif "locks" in line:
                        lst.insert(
                            next_line, self.CreateStruct(False))

            fout.writelines(lst)

    def CreateIds(self) -> str:
        """Create lock IDs for header file"""
        ids = []
        for name, id in zip(self.name, self.id):
            result = f"#define LOCKING_ID_{name}".ljust(ID_WIDTH) + str(id) + "\n"
            ids.append(result)
        return ''.join(ids)

    def CreateConstants(self) -> str:
        """Create some definitions for header file"""
        defs = []

        self.MaxNameLength = len(max(self.name, key=len))

        defs.append(self.JustifyDefine(
            "TABLE_SIZE", "", self.projectLocksCount))
        defs.append(self.JustifyDefine(
            "TABLE_MAX_ID", "", max(self.id)))

        return ''.join(defs)

    def JustifyDefine(self, key: str, suffix: str, value: int) -> str:
        width = self.MaxNameLength + DEFINE_WIDTH
        if len(suffix) != 0:
            name = key + "_" + suffix
        else:
            name = key
        return "#define LOCKING_" + name.ljust(width) + f" {str(value)}\n"

    def _CreateLockHeaderFile(self, lst: list) -> None:
        """Create the locks header file"""
        name = HEADER_FILE_PATH + TABLE_FILE_NAME + ".h"
        print("Writing " + name)
        with open(name, 'w') as fout:
            for index, line in enumerate(lst):
                next_line = index + 1
                if "pystart - " in line:
                    if "locking ids" in line:
                        lst.insert(next_line, self.CreateIds())
                    elif "locking constants" in line:
                        lst.insert(next_line, self.CreateConstants())

            fout.writelines(lst)


if __name__ == "__main__":
    file_name = "./lockings.json"
    if ((len(sys.argv)-1)) == 1:
        project = sys.argv[1]
    elif ((len(sys.argv)-1)) == 2:
        project = sys.argv[1]
        file_name = sys.argv[2]
    else:
        project = "MG100"

    # Add project name to paths
    BASE_FILE_PATH = BASE_FILE_PATH.replace("%PROJ%", project)
    HEADER_FILE_PATH = HEADER_FILE_PATH.replace("%BASE%", BASE_FILE_PATH)
    SOURCE_FILE_PATH = SOURCE_FILE_PATH.replace("%BASE%", BASE_FILE_PATH)

    # Ensure path directories exists, else create them
    if (not os.path.isdir(BASE_FILE_PATH)):
        os.mkdir(BASE_FILE_PATH)
        print("Created base folder for project " +
              project + " at " + BASE_FILE_PATH)
    if (not os.path.isdir(HEADER_FILE_PATH)):
        os.mkdir(HEADER_FILE_PATH)
        print("Created header folder for project " +
              project + " at " + HEADER_FILE_PATH)
    if (not os.path.isdir(SOURCE_FILE_PATH)):
        os.mkdir(SOURCE_FILE_PATH)
        print("Created source folder for project " +
              project + " at " + SOURCE_FILE_PATH)

    # Ensure .h and .c file exist
    if (not os.path.exists(HEADER_FILE_PATH + TABLE_FILE_NAME + ".h")):
        raise Exception("Missing header file for project " + project +
                        " at " + HEADER_FILE_PATH + TABLE_FILE_NAME + ".h")
    if (not os.path.exists(SOURCE_FILE_PATH + TABLE_FILE_NAME + ".c")):
        raise Exception("Missing source file for project " + project +
                        " at " + SOURCE_FILE_PATH + TABLE_FILE_NAME + ".c")

    # Parse locks
    a = locks(project, file_name)

    a.UpdateFiles()
