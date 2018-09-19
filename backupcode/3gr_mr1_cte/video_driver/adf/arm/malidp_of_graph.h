/*
 * OF graph binding parsing helpers
 *
 * (C) COPYRIGHT 2014 ARM Limited. All rights reserved.
 *
 * Copyright (C) 2012 - 2013 Samsung Electronics Co., Ltd.
 * Author: Sylwester Nawrocki <s.nawrocki@samsung.com>
 *
 * Copyright (C) 2012 Renesas Electronics Corp.
 * Author: Guennadi Liakhovetski <g.liakhovetski@gmx.de>
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained
 * from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 * This is a partial direct copy of include/linux/of_graph.h from Linux 3.15
 */
#ifndef __LINUX_OF_GRAPH_H
#define __LINUX_OF_GRAPH_H

#ifdef CONFIG_OF
struct device_node *of_graph_get_next_endpoint(const struct device_node *parent,
					struct device_node *previous);
struct device_node *of_graph_get_remote_port_parent(
					const struct device_node *node);
#else

static inline struct device_node *of_graph_get_next_endpoint(
					const struct device_node *parent,
					struct device_node *previous)
{
	return NULL;
}

static inline struct device_node *of_graph_get_remote_port_parent(
					const struct device_node *node)
{
	return NULL;
}

#endif /* CONFIG_OF */

#endif /* __LINUX_OF_GRAPH_H */
