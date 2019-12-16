#!/usr/bin/python3
import sys
import getopt
import os
from enum import Enum
from enum import unique
from rmi import rmi


@unique
class AnnotationType(Enum):
    ANNOTATION_NONE = 0
    ANNOTATION_RMI = 1


def print_help():
    help_str = ('-f, --filename\n'
                '-r, --rmi: 为rmi标记的方法生成对应RMI代码\n')

    return help_str


def main(argv=None):
    if argv is None:
        argv = sys.argv

    annotation = AnnotationType.ANNOTATION_NONE
    filepath = ""
    opts, args = getopt.getopt(argv[1:], "-h-f:-r", ['help', 'filename=', 'rmi'])
    for opt_name, opt_val in opts:
        if opt_name in ('-h', "--help"):
            print(print_help())
            return 0

        '''
        为rmi接口生成对应代码
        '''
        if opt_name in ('-r', "--rmi"):
            annotation = AnnotationType.ANNOTATION_RMI

        if opt_name in ('-f', '--filename'):
            filepath = opt_val

    if annotation == AnnotationType.ANNOTATION_NONE:
        print(print_help())
        return 1

    if not os.path.exists(filepath):
        print(filepath + " not exists. please check it!")
        return 2

    if annotation == AnnotationType.ANNOTATION_RMI:
        return rmi.annotation_parser_rmi(filepath)
    return 0


if __name__ == "__main__":
    sys.exit(main())
