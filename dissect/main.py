#!/usr/bin/env python
import dissect

GRAPHS = {
    # a -> b -> d;
    # a -> c -> d;
    'diamond': [
        'Va/Vb/VdEE/Vc/VdEEE',
        'Vb/VdEE',
        'Vc/VdEE',
        'VdE'
    ],

    # a -> b -> c -> b;
    # a -> d -> b;
    'twice visited loop': [
        "Va/Vb/Vc/R1EE/Vd/R2EE",
        "Vb/Vc/R1EE",
        "Vc/Vb/R1EE",
        "Vd/Vb/Vc/R1EEE"
    ],

    # a -> b -> a;
    # a -> c -> b;
    'double loop': [
        'Va/Vb/R1E/Vc/R1EE',
        'Vb/Va/R1/Vc/R2EEE',
        'Vc/Vb/Va/R1/R2EEE'
    ],

    # a -> b -> c -> b;
    # a -> d -> e;
    # d -> f;
    'cyclic and acyclic paths': [
        'Va/Vb/Vc/R1EE/Vd/VeE/VfEEE',
        'Vb/Vc/R1EE',
        'Vc/Vb/R1EE',
        'Vd/VeE/VfEE',
        'VeE',
        'VfE'
    ],

    # a -> b -> a;
    # c -> d;
    'two islands': [
        'Va/Vb/R1EE',
        'Vb/Va/R1EE',
        'Vc/VdEE',
        'VdE'
    ]
}

if __name__ == '__main__':
    dissect.print_graph(GRAPHS['twice visited loop'], '1')
