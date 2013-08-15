/* ui-patch.c: generate patch view
 *
 * Copyright (C) 2007 Lars Hjemli
 *
 * Licensed under GNU General Public License v2
 *   (see COPYING for full license text)
 */

#include "cgit.h"
#include "ui-patch.h"
#include "html.h"
#include "ui-shared.h"

void cgit_print_patch(char *hex, const char *old_rev, const char *prefix)
{
	struct rev_info rev;
	struct commit *commit;
	unsigned char sha1[20], old_sha1[20];
	char rev_range[2 * 40 + 3];
	char *rev_argv[] = { NULL, "--reverse", rev_range };
	char *patchname;

	if (!hex)
		hex = ctx.qry.head;

	if (get_sha1(hex, sha1)) {
		cgit_print_error("Bad object id: %s", hex);
		return;
	}
	commit = lookup_commit_reference(sha1);
	if (!commit) {
		cgit_print_error("Bad commit reference: %s", hex);
		return;
	}

	if (old_rev) {
		if (get_sha1(old_rev, old_sha1)) {
			cgit_print_error("Bad object id: %s", old_rev);
			return;
		}
		if (!lookup_commit_reference(old_sha1)) {
			cgit_print_error("Bad commit reference: %s", old_rev);
			return;
		}
	} else if (commit->parents && commit->parents->item) {
		hashcpy(old_sha1, commit->parents->item->object.sha1);
	} else {
		hashclr(old_sha1);
	}

	sprintf(rev_range, "%s..%s", sha1_to_hex(old_sha1), sha1_to_hex(sha1));

	patchname = fmt("%s.patch", sha1_to_hex(sha1));
	ctx.page.mimetype = "text/plain";
	ctx.page.filename = patchname;
	cgit_print_http_headers(&ctx);

	init_revisions(&rev, NULL);
	rev.abbrev = DEFAULT_ABBREV;
	rev.commit_format = CMIT_FMT_EMAIL;
	rev.verbose_header = 1;
	rev.diff = 1;
	rev.show_root_diff = 0;
	rev.diffopt.output_format |= DIFF_FORMAT_PATCH;
	setup_revisions(ARRAY_SIZE(rev_argv), (const char **)rev_argv, &rev,
			NULL);
	prepare_revision_walk(&rev);

	while ((commit = get_revision(&rev)) != NULL) {
		log_tree_commit(&rev, commit);
		printf("--\ncgit %s\n", cgit_version);
	}
}
