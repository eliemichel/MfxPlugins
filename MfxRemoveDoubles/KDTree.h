/**
 * This file is part of MfxPlugins
 *
 * Copyright (c) 2019 -- Élie Michel <elie.michel@exppad.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * The Software is provided “as is”, without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose and non-infringement. In no event shall the
 * authors or copyright holders be liable for any claim, damages or other
 * liability, whether in an action of contract, tort or otherwise, arising
 * from, out of or in connection with the software or the use or other dealings
 * in the Software.
 */

#pragma once

#include <vector>

struct kd_node_t{
	char* p; // pointer to data pool
    struct kd_node_t *left, *right;
};

class KDTree {
public:
	using Real = float;

    KDTree(int point_count, char*point_data, int stride = 3);

	/**
	 * Get the nearest point (useless as is, it will return the target point
	 * itself since by construction it is in the tree)
	 */
    void nearest(int point_index, int & best_index, Real & best_distance);

	/**
	 * Get point with the minimum index among the points lying within a given
	 * radius around target.
	 */
	int equivalent(int point_index, Real radius);

private:
	char *m_point_data;
    std::vector<struct kd_node_t> m_nodes;
    struct kd_node_t *m_root;
    int m_stride;
};
