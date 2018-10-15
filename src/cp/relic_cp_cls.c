/*
 * RELIC is an Efficient LIbrary for Cryptography
 * Copyright (C) 2007-2017 RELIC Authors
 *
 * This file is part of RELIC. RELIC is legal property of its developers,
 * whose names are not listed here. Please refer to the COPYRIGHT file
 * for contact information.
 *
 * RELIC is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * RELIC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RELIC. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 *
 * Implementation of the Camenisch-Lysyanskaya signature protocols.
 *
 * @ingroup cp
 */

#include "relic.h"

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

int cp_cls_gen(bn_t r, bn_t s, g2_t x, g2_t y) {
	bn_t n;
	int result = STS_OK;

	bn_null(n);

	TRY {
		bn_new(n);

		g2_get_ord(n);
		bn_rand_mod(r, n);
		bn_rand_mod(s, n);
		g2_mul_gen(x, r);
		g2_mul_gen(y, s);
	}
	CATCH_ANY {
		result = STS_ERR;
	}
	FINALLY {
		bn_free(n);
	}
	return result;
}

int cp_cls_sig(g1_t a, g1_t b, g1_t c, uint8_t *msg, int len, bn_t r, bn_t s) {
	bn_t m, n;
	int result = STS_OK;

	bn_null(m);
	bn_null(n);

	TRY {
		bn_new(m);
		bn_new(n);

		g1_rand(a);
		g1_get_ord(n);
		bn_mul(m, r, s);
		bn_mod(m, m, n);
		g1_mul(b, a, s);
		bn_read_bin(m, msg, len);
		bn_mul(m, m, s);
		bn_mul(m, m, r);
		bn_add(m, m, r);
		bn_mod(m, m, n);
		g1_mul(c, a, m);
	}
	CATCH_ANY {
		result = STS_ERR;
	}
	FINALLY {
		bn_free(m);
		bn_free(n);
	}
	return result;
}

int cp_cls_ver(g1_t a, g1_t b, g1_t c, uint8_t *msg, int len, g2_t x, g2_t y) {
	g1_t p[2];
	g2_t r[2];
	gt_t e;
	bn_t m, n;
	int result = 0;

	g1_null(p[0]);
	g1_null(p[1]);
	g2_null(r[0]);
	g2_null(r[1]);
	gt_null(e);
	bn_null(m);
	bn_null(n);

	TRY {
		g1_new(p[0]);
		g1_new(p[1]);
		g2_new(r[0]);
		g2_new(r[1]);
		gt_new(e);
		bn_new(m);
		bn_new(n);

		g1_copy(p[0], a);
		g1_copy(p[1], b);
		g2_copy(r[0], y);
		g2_get_gen(r[1]);
		g2_neg(r[1], r[1]);

		pc_map_sim(e, p, r, 2);
		if (gt_is_unity(e)) {
			result = 1;
		}

		g1_get_ord(n);
		bn_read_bin(m, msg, len);
		bn_mod(m, m, n);
		g1_mul(p[0], b, m);
		g1_add(p[0], p[0], a);
		g1_norm(p[0], p[0]);
		g1_copy(p[1], c);
		g2_copy(r[0], x);

		pc_map_sim(e, p, r, 2);
		if (result == 1 && !gt_is_unity(e)) {
			result = 0;
		}
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		g1_free(p[0]);
		g1_free(p[1]);
		g2_free(r[0]);
		g2_free(r[1]);
		gt_free(e);
		bn_free(m);
		bn_free(n);
	}
	return result;
}

int cp_cli_gen(bn_t t, bn_t u, bn_t v, g2_t x, g2_t y, g2_t z) {
	bn_t n;
	int result = STS_OK;

	bn_null(n);

	TRY {
		bn_new(n);

		g2_get_ord(n);
		bn_rand_mod(t, n);
		bn_rand_mod(u, n);
		bn_rand_mod(v, n);
		g2_mul_gen(x, t);
		g2_mul_gen(y, u);
		g2_mul_gen(z, v);
	}
	CATCH_ANY {
		result = STS_ERR;
	}
	FINALLY {
		bn_free(n);
	}
	return result;
}

int cp_cli_sig(g1_t a, g1_t A, g1_t b, g1_t B, g1_t c, uint8_t *msg, int len,
		bn_t r, bn_t t, bn_t u, bn_t v) {
	bn_t m, n;
	int result = STS_OK;

	bn_null(m);
	bn_null(n);

	TRY {
		bn_new(m);
		bn_new(n);

		/* Choose random a in G1. */
		g1_rand(a);
		/* Compute A = a^z, B = A^y. */
		g1_mul(A, a, v);
		g1_mul(b, a, u);
		g1_mul(B, A, u);
		/* Compute c = A^(xyr) = B^{xr}. */
		g1_get_ord(n);
		bn_mul(m, t, r);
		bn_mod(m, m, n);
		g1_mul(b, B, m);
		/* Compute c = a^{x+xym}A^(xyr). */
		bn_read_bin(m, msg, len);
		bn_mul(m, m, t);
		bn_mul(m, m, u);
		bn_add(m, m, t);
		bn_mod(m, m, n);
		g1_mul(c, a, m);
		g1_add(c, c, b);
		g1_norm(c, c);
		/* Compute b = a^y. */
		g1_mul(b, a, u);
	}
	CATCH_ANY {
		result = STS_ERR;
	}
	FINALLY {
		bn_free(m);
		bn_free(n);
	}
	return result;
}

int cp_cli_ver(g1_t a, g1_t A, g1_t b, g1_t B, g1_t c, uint8_t *msg, int len,
		bn_t r, g2_t x, g2_t y, g2_t z) {
	g1_t p[2];
	g2_t q[2];
	gt_t e;
	bn_t m, n;
	int result = 0;

	g1_null(p[0]);
	g1_null(p[1]);
	g2_null(q[0]);
	g2_null(q[1]);
	gt_null(e);
	bn_null(m);
	bn_null(n);

	TRY {
		g1_new(p[0]);
		g1_new(p[1]);
		g2_new(q[0]);
		g2_new(q[1]);
		gt_new(e);
		bn_new(m);
		bn_new(n);

		/* Check that e(a, Z) = e(A, g) or that e(a, Z)e(A, -g) = 1. */
		g1_copy(p[0], a);
		g1_copy(p[1], A);
		g2_copy(q[0], z);
		g2_get_gen(q[1]);
		g2_neg(q[1], q[1]);
		pc_map_sim(e, p, q, 2);
		if (gt_is_unity(e)) {
			result = 1;
		}

		/* Check e(a, Y) = e(b, g) and e(A, Y) = e(B, g) using same trick. */
		g1_copy(p[1], b);
		g2_copy(q[0], y);
		pc_map_sim(e, p, q, 2);
		if (result == 1 && !gt_is_unity(e)) {
			result = 0;
		}
		g1_copy(p[0], A);
		g1_copy(p[1], B);
		pc_map_sim(e, p, q, 2);
		if (result == 1 && !gt_is_unity(e)) {
			result = 0;
		}

		/* Check that e(a, X)e(mb, X)e(rB, X) = e(c, g). */
		g1_get_ord(n);
		bn_read_bin(m, msg, len);
		bn_mod(m, m, n);
		g1_mul(p[0], b, m);
		g1_add(p[0], p[0], a);
		g1_mul(p[1], B, r);
		g1_add(p[0], p[0], p[1]);
		g1_norm(p[0], p[0]);
		g1_copy(p[1], c);
		g2_copy(q[0], x);
		pc_map_sim(e, p, q, 2);
		if (result == 1 && !gt_is_unity(e)) {
			result = 0;
		}
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		g1_free(p[0]);
		g1_free(p[1]);
		g2_free(q[0]);
		g2_free(q[1]);
		gt_free(e);
		bn_free(m);
		bn_free(n);
	}
	return result;
}
