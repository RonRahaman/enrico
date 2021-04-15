#!/usr/bin/env python3

import xml.etree.ElementTree as ET


def remove_attribs_from_elem(tree, elem_tag, attribs):
    for elem in tree.iter(tag=elem_tag):
        for a in attribs:
            elem.attrib.pop(a, None)


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("xml_file")
    args = parser.parse_args()

    junit = ET.parse(args.xml_file)
    remove_attribs_from_elem(junit, "testsuite", ["skips"])
    remove_attribs_from_elem(junit, "testcase", ["file", "line"])
    junit.write(args.xml_file)
