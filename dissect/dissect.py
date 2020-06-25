#!/usr/bin/env python
from __future__ import print_function
import re


def _vertex(s, track, stack, graph):
    """
    Processes a vertex definition from the input string s.

    :param s: The input string.
    :param track: A sequence of the vertices so far defined.
    :param stack: A stack of the vertices on the current path. An 'E' command pops the top-most vertex.
    :param graph: A set of vertex pairs which make up the graph.
    :return: A set of vertex pairs which make up the graph.
    """

    # A vertex name is a sequence of lower-case letters.
    match = re.match(r'[a-z]+', s)
    if match is None:
        raise RuntimeError('No vertex name found')

    vertex_name = match.group(0)
    stack.append(vertex_name)  # push

    # The stack contains the path that was followed to arrive at this vertex. The track on the other
    # hand is a list of all of the vertices that have been defined: it's used when a back-reference
    # wants to point to some previous vertex.
    track.append(vertex_name)

    if len(stack) > 1:
        predecessor = stack[-2]
        # print('%s -> %s;' % (predecessor, v))
        graph.add((predecessor, vertex_name))

    return _dissect(s[len(vertex_name):], track, stack, graph)


def _edge(s, track, stack, graph):
    return _dissect(s, track, stack, graph)



def _end(s, track, stack, graph):
    del stack[-1]  # pop
    return _dissect(s, track, stack, graph)


def _get_number(s, init=0):
    """
    Extracts a sequence zero of more digits from the front of string 's' and converts them to the
    integer equivalent. The process stops when the string is exhausted or the first non-digit
    character is encountered.

    For example, _get_number('123hello') will yield the tuple (123, 'hello').

    :param s: A string starting with zero of more digits.
    :param init: The initial integer value.
    :return: A tuple containing the remaining part of the string and the integer value.
    """

    return _get_number(s[1:], init * 10 + int(s[0])) if len(s) > 0 and s[0].isdigit() else (s, init)


def _back_reference(s, track, stack, graph):
    rest, num = _get_number(s)
    back_edge = track[-(num + 1)]
    last_vertex = stack[-1]
    # print("%s -> %s;" % (last_vertex, back_edge))
    graph.add((last_vertex, back_edge))
    return _dissect(rest, track, stack, graph)


def _unknown_token(*_):
    raise RuntimeError('Unknown Token')


def _dissect(s, track, stack, graph):
    """
    Decodes a path through a cyclic directed graph as described by the hash string s.

    :param s: The input string.
    :param track: A sequence of the vertices so far defined.
    :param stack: A stack of the vertices met on the current path.
    :param graph: A set of vertex pairs which make up the graph.
    :return: A set of vertex pairs which make up the graph.
    """

    if len(s) == 0:
        return graph
    return {
        '/': _edge,
        'E': _end,  # An 'E' command marks the end of a vertex's edges.
        'R': _back_reference,
        'V': _vertex  # A 'V' command defines a vertex.
    }.get(s[0], _unknown_token)(s[1:], track, stack, graph)


def dissect(s, graph):
    """
    Decodes a path through a cyclic directed graph as described by the hash string s.

    :param s: The input string.
    :param graph: A set of vertex pairs which make up the graph.
    :return: A set of vertex pairs which make up the graph.
    """

    return _dissect(s, track=[], stack=[], graph=graph)


def print_graph(src, suffix=''):
    """
    Converts an iterable of hash strings to a GraphViz DOT description of the original graph.

    :param src: An array of graph hash strings.
    :param suffix: A string appended to each vertex name.
    :return: None
    """

    g = set()
    for i in src:
        # print('dissecting: ', i)
        dissect(i, graph=g)
    print ('digraph G {')
    for edge in sorted([e for e in g], key=lambda k: k[0] + ' ' + k[1]):
        print('    %s%s -> %s%s;' % (edge[0], suffix, edge[1], suffix))
    print ('}')
