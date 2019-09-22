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

#include <iostream>
#include <algorithm>
#include "KDTree.h"

// Core code is from https://rosettacode.org/wiki/K-d_tree
// It is actually very buggy, I should look for another one.
// The tree is not well built when there are several points with the exact same
// coordinate on one axis, because it uses the value to recognize the point
// The 'equivalent' functino is my own, and is buggy as well. It does not
// always find the quaried point itself, and is not idempotent while it should
// (ie ensure that equivalent(equivalent(p)) = equivalent(p) for any p)

/**
 * Squared euclidian distance between the roots of two trees
 */
static inline Real dist(struct kd_node_t *a, struct kd_node_t *b)
{
    Real dx = a->p[0] - b->p[0];
    Real dy = a->p[1] - b->p[1];
    Real dz = a->p[2] - b->p[2];
    return dx * dx + dy * dy + dz * dz;
}

/**
 * Swap node values only (not node children)
 */
static inline void swap(struct kd_node_t *x, struct kd_node_t *y) {
    Real *tmp = x->p;
    x->p = y->p;
    y->p = tmp;
}

/**
 * From a range of node pointers find the median one with regard to its dimension axis
 */
static struct kd_node_t* find_median(struct kd_node_t *start, struct kd_node_t *end, int axis)
{
    if (end <= start) return NULL;
    if (end == start + 1)
        return start;
 
    struct kd_node_t *p, *store, *md = start + (end - start) / 2;
    double pivot;
    while (1) {
        pivot = md->p[axis];
 
        swap(md, end - 1);
        for (store = p = start; p < end; p++) {
            if (p->p[axis] < pivot) {
                if (p != store)
                    swap(p, store);
                store++;
            }
        }
        swap(store, end - 1);
 
        /* TODO: PROBLEM if median has duplicate values */
        if (store->p[axis] == md->p[axis])
            return md;
 
        if (store > md) end = store;
        else        start = store;
    }
}
 
static struct kd_node_t* make_tree(struct kd_node_t *t, int len, int axis = 0)
{
    struct kd_node_t *n;
 
    if (!len) return 0;
 
    if (n = find_median(t, t + len, axis)) {
		axis = (axis + 1) % 3;
		n->left  = make_tree(t, n - t, axis);
		n->right = make_tree(n + 1, t + len - (n + 1), axis);
    }
    return n;
}

static void nearest_aux(struct kd_node_t *root, struct kd_node_t *target,
        struct kd_node_t **best, Real *best_dist, int axis = 0)
{
    Real d, dx, dx2;
 
    if (!root) return;
    d = dist(root, target);
    dx = root->p[axis] - target->p[axis];
    dx2 = dx * dx;

    if (!*best || d < *best_dist) {
        *best_dist = d;
        *best = root;
    }
 
    /* if chance of exact match is high */
    if (!*best_dist) return;
 
    axis = (axis + 1) % 3;
 
    nearest_aux(dx > 0 ? root->left : root->right, target, best, best_dist, axis);
    if (dx2 > *best_dist) return;
    nearest_aux(dx > 0 ? root->right : root->left, target, best, best_dist, axis);
}

/**
 * Get point with the minimum index among the points lying within a given
 * radius around target.
 */
static void equivalent_aux(struct kd_node_t *root, struct kd_node_t *target,
                          Real sqradius, struct kd_node_t **best, int axis = 0)
{
    Real d, dx, dx2;
 
    if (!root) return;
    d = dist(root, target);
    dx = root->p[axis] - target->p[axis];
    dx2 = dx * dx;

	if (d <= sqradius && (!*best || root->p < (*best)->p)) {
		*best = root;
	}
 
    axis = (axis + 1) % 3;
 
	equivalent_aux(dx > 0 ? root->left : root->right, target, sqradius, best, axis);
    if (dx2 > sqradius) return;
	equivalent_aux(dx > 0 ? root->right : root->left, target, sqradius, best, axis);
}

static void print_aux(struct kd_node_t *root, int offset = 0)
{
    if (!root) return;
	for (int i = 0 ; i < offset ; ++i) std::cout << "  ";
	std::cout << "[" << root->p[0] << ", " << root->p[1] << ", " << root->p[2] << "]" << std::endl;
    print_aux(root->left, offset + 1);
	print_aux(root->right, offset + 1);
}

KDTree::KDTree(int point_count, Real *point_data, int stride)
    : m_point_data(point_data)
	, m_nodes(point_count)
    , m_stride(stride)
{
    for (int i = 0 ; i < point_count ; ++i) {
        m_nodes[i].p = point_data + m_stride * i;
    }
    m_root = make_tree(m_nodes.data(), static_cast<int>(m_nodes.size()));
}

void KDTree::nearest(int point_index, int & best_index, Real & best_distance) {
    struct kd_node_t *best = NULL;
	struct kd_node_t target;
	target.p = m_point_data + m_stride * point_index;
    nearest_aux(m_root, &target, &best, &best_distance);
    best_index = NULL == best ? -1 : (best->p - m_point_data) / m_stride;
}

int KDTree::equivalent(int point_index, Real radius) {
    struct kd_node_t *best = NULL;
	struct kd_node_t target;
	target.p = m_point_data + m_stride * point_index;
	equivalent_aux(m_root, &target, radius * radius, &best);
	// The min should not be needed if the algorithm would work
    return NULL == best ? point_index : std::min(point_index, static_cast<int>((best->p - m_point_data) / m_stride));
}
