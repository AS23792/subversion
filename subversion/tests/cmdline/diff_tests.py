import sys, re, os, time, shutil
from svntest import err
def is_absolute_url(target):
  return (target.startswith('file://')
          or target.startswith('http://')
          or target.startswith('https://')
          or target.startswith('svn://')
          or target.startswith('svn+ssh://'))

def make_diff_header(path, old_tag, new_tag, src_label=None, dst_label=None):
  """Generate the expected diff header for file PATH, with its old and new
  versions described in parentheses by OLD_TAG and NEW_TAG. SRC_LABEL and
  DST_LABEL are paths or urls that are added to the diff labels if we're
  diffing against the repository or diffing two arbitrary paths.
  Return the header as an array of newline-terminated strings."""
  if src_label:
    src_label = src_label.replace('\\', '/')
    if not is_absolute_url(src_label):
      src_label = '.../' + src_label
    src_label = '\t(' + src_label + ')'
  else:
    src_label = ''
  if dst_label:
    dst_label = dst_label.replace('\\', '/')
    if not is_absolute_url(dst_label):
      dst_label = '.../' + dst_label
    dst_label = '\t(' + dst_label + ')'
  else:
    dst_label = ''
  path_as_shown = path.replace('\\', '/')
  return [
    "Index: " + path_as_shown + "\n",
    "===================================================================\n",
    "--- " + path_as_shown + src_label + "\t(" + old_tag + ")\n",
    "+++ " + path_as_shown + dst_label + "\t(" + new_tag + ")\n",
    ]

def make_no_diff_deleted_header(path, old_tag, new_tag):
  """Generate the expected diff header for a deleted file PATH when in
  'no-diff-deleted' mode. (In that mode, no further details appear after the
  header.) Return the header as an array of newline-terminated strings."""
  path_as_shown = path.replace('\\', '/')
  return [
    "Index: " + path_as_shown + " (deleted)\n",
    "===================================================================\n",
    ]

def make_git_diff_header(target_path, repos_relpath,
                         old_tag, new_tag, add=False, src_label=None,
                         dst_label=None, delete=False, text_changes=True,
                         cp=False, mv=False, copyfrom_path=None):
  """ Generate the expected 'git diff' header for file TARGET_PATH.
  REPOS_RELPATH is the location of the path relative to the repository root.
  The old and new versions ("revision X", or "working copy") must be
  specified in OLD_TAG and NEW_TAG.
  SRC_LABEL and DST_LABEL are paths or urls that are added to the diff
  labels if we're diffing against the repository. ADD, DELETE, CP and MV
  denotes the operations performed on the file. COPYFROM_PATH is the source
  of a copy or move.  Return the header as an array of newline-terminated
  strings."""

  path_as_shown = target_path.replace('\\', '/')
  if src_label:
    src_label = src_label.replace('\\', '/')
    src_label = '\t(.../' + src_label + ')'
  else:
    src_label = ''
  if dst_label:
    dst_label = dst_label.replace('\\', '/')
    dst_label = '\t(.../' + dst_label + ')'
  else:
    dst_label = ''

  if add:
    output = [
      "Index: " + path_as_shown + "\n",
      "===================================================================\n",
      "diff --git a/" + repos_relpath + " b/" + repos_relpath + "\n",
      "new file mode 10644\n",
    ]
    if text_changes:
      output.extend([
        "--- /dev/null\t(" + old_tag + ")\n",
        "+++ b/" + repos_relpath + dst_label + "\t(" + new_tag + ")\n"
      ])
  elif delete:
    output = [
      "Index: " + path_as_shown + "\n",
      "===================================================================\n",
      "diff --git a/" + repos_relpath + " b/" + repos_relpath + "\n",
      "deleted file mode 10644\n",
    ]
    if text_changes:
      output.extend([
        "--- a/" + repos_relpath + src_label + "\t(" + old_tag + ")\n",
        "+++ /dev/null\t(" + new_tag + ")\n"
      ])
  elif cp:
    output = [
      "Index: " + path_as_shown + "\n",
      "===================================================================\n",
      "diff --git a/" + copyfrom_path + " b/" + repos_relpath + "\n",
      "copy from " + copyfrom_path + "\n",
      "copy to " + repos_relpath + "\n",
    ]
    if text_changes:
      output.extend([
        "--- a/" + copyfrom_path + src_label + "\t(" + old_tag + ")\n",
        "+++ b/" + repos_relpath + "\t(" + new_tag + ")\n"
      ])
  elif mv:
    return [
      "Index: " + path_as_shown + "\n",
      "===================================================================\n",
      "diff --git a/" + copyfrom_path + " b/" + path_as_shown + "\n",
      "rename from " + copyfrom_path + "\n",
      "rename to " + repos_relpath + "\n",
    ]
    if text_changes:
      output.extend([
        "--- a/" + copyfrom_path + src_label + "\t(" + old_tag + ")\n",
        "+++ b/" + repos_relpath + "\t(" + new_tag + ")\n"
      ])
  else:
    output = [
      "Index: " + path_as_shown + "\n",
      "===================================================================\n",
      "diff --git a/" + repos_relpath + " b/" + repos_relpath + "\n",
      "--- a/" + repos_relpath + src_label + "\t(" + old_tag + ")\n",
      "+++ b/" + repos_relpath + dst_label + "\t(" + new_tag + ")\n",
    ]
  return output

def make_diff_prop_header(path):
  """Return a property diff sub-header, as a list of newline-terminated
     strings."""
  return [
    "\n",
    "Property changes on: " + path.replace('\\', '/') + "\n",
    "___________________________________________________________________\n"
  ]

def make_diff_prop_val(plus_minus, pval):
  "Return diff for prop value PVAL, with leading PLUS_MINUS (+ or -)."
  if len(pval) > 0 and pval[-1] != '\n':
    return [plus_minus + pval + "\n","\\ No newline at end of property\n"]
  return [plus_minus + pval]
  
def make_diff_prop_deleted(pname, pval):
  """Return a property diff for deletion of property PNAME, old value PVAL.
     PVAL is a single string with no embedded newlines.  Return the result
     as a list of newline-terminated strings."""
  return [
    "Deleted: " + pname + "\n",
    "## -1 +0,0 ##\n"
  ] + make_diff_prop_val("-", pval)

def make_diff_prop_added(pname, pval):
  """Return a property diff for addition of property PNAME, new value PVAL.
     PVAL is a single string with no embedded newlines.  Return the result
     as a list of newline-terminated strings."""
  return [
    "Added: " + pname + "\n",
    "## -0,0 +1 ##\n",
  ] + make_diff_prop_val("+", pval)

def make_diff_prop_modified(pname, pval1, pval2):
  """Return a property diff for modification of property PNAME, old value
     PVAL1, new value PVAL2.  PVAL is a single string with no embedded
     newlines.  Return the result as a list of newline-terminated strings."""
  return [
    "Modified: " + pname + "\n",
    "## -1 +1 ##\n",
  ] + make_diff_prop_val("-", pval1) + make_diff_prop_val("+", pval2)
      print('Sought: %s' % excluded)
      print('Found:  %s' % line)
    None, 'diff', '-r', '1', os.path.join(wc_dir, 'A', 'D'))
    None, 'diff', '-r', '1', '-N', os.path.join(wc_dir, 'A', 'D'))
    None, 'diff', '-r', '1', os.path.join(wc_dir, 'A', 'D', 'G'))
  svntest.main.file_append(os.path.join(wc_dir, 'A', 'D', 'foo'), "a new file")
  exit_code, diff_output, err_output = svntest.main.run_svn(
    1, 'diff', os.path.join(wc_dir, 'A', 'D', 'foo'))

  if count_diff_output(diff_output) != 0: raise svntest.Failure

  # At one point this would crash, so we would only get a 'Segmentation Fault'
  # error message.  The appropriate response is a few lines of errors.  I wish
  # there was a way to figure out if svn crashed, but all run_svn gives us is
  # the output, so here we are...
  for line in err_output:
    if re.search("foo' is not under version control$", line):
      break
  else:
    raise svntest.Failure
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, expected_output, [],
  svntest.actions.run_and_verify_svn(None, expected_output, [],
  svntest.actions.run_and_verify_svn(None, expected_reverse_output, [],
  svntest.actions.run_and_verify_svn(None, expected_reverse_output, [],
  svntest.actions.run_and_verify_svn(None, expected_rev1_output, [],
  svntest.actions.run_and_verify_svn(None, expected_rev1_output, [],
  theta_path = os.path.join(wc_dir, 'A', 'theta')
                                        expected_status, None, wc_dir)
                                        None, None, None, None, None,
                                        1)  # verify props, too.
                                        expected_status, None, wc_dir)
  mu_path = os.path.join(sbox.wc_dir, 'A', 'mu')
  svntest.actions.run_and_verify_svn(None, svntest.verify.AnyOutput, [],
  iota_path = os.path.join(sbox.wc_dir, 'iota')
  newfile_path = os.path.join(sbox.wc_dir, 'A', 'D', 'newfile')
  mu_path = os.path.join(sbox.wc_dir, 'A', 'mu')
                                        expected_status, None, wc_dir)
    None, None, [], 'diff', '-r', '1', wc_dir)
    None, None, [], 'diff', '-r', 'BASE:1', wc_dir)
    None, None, [], 'diff', '-r', '2:1', wc_dir)
    None, None, [], 'diff', '-r', '1:2', wc_dir)
    None, None, [], 'diff', '-r', '1:BASE', wc_dir)
    None, None, [], 'diff', '-r', '1:2', wc_dir)
    None, None, [], 'diff', '-r', '1:BASE', wc_dir)
    None, None, [], 'diff', '-r', '2:1', wc_dir)
    None, None, [], 'diff', '-r', 'BASE:1', wc_dir)
                                        expected_status, None, wc_dir)
    None, None, [], 'diff', '-r', '3:2', wc_dir)
    None, None, [], 'diff', '-r', 'BASE:2', wc_dir)
  A_path = os.path.join(sbox.wc_dir, 'A')
  mu_path = os.path.join(sbox.wc_dir, 'A', 'mu')
                                        expected_status, None, wc_dir)
                                        expected_status, None, wc_dir)
  diff_output = svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  A_alpha = os.path.join(sbox.wc_dir, 'A', 'B', 'E', 'alpha')
  A2_alpha = os.path.join(sbox.wc_dir, 'A2', 'B', 'E', 'alpha')
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
    None, None, [], 'diff', '--old', A_url, '--new', A2_url, rel_path)
    None, None, [], 'diff', '--old', A_url, '--new', A2_url)
    None, None, [],
    None, None, [],
    None, [], [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  A_alpha = os.path.join(sbox.wc_dir, 'A', 'B', 'E', 'alpha')
  A2_alpha = os.path.join(sbox.wc_dir, 'A2', 'B', 'E', 'alpha')
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  A_path = os.path.join(sbox.wc_dir, 'A')
    None, None, [],
    None, None, [],
  iota_path = os.path.join(sbox.wc_dir, 'iota')
  iota_copy_path = os.path.join(sbox.wc_dir, 'A', 'iota')
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  exit_code, out, err = svntest.actions.run_and_verify_svn(None, None, [],
  exit_code, out, err = svntest.actions.run_and_verify_svn(None, None, [],
  iota_path = os.path.join(sbox.wc_dir, 'iota')
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  exit_code, out, err = svntest.actions.run_and_verify_svn(None, None, [],
  exit_code, out, err = svntest.actions.run_and_verify_svn(None, None, [],
  exit_code, out, err = svntest.actions.run_and_verify_svn(None, None, [],
  prefix_path = os.path.join(sbox.wc_dir, 'prefix_mydir')
  svntest.actions.run_and_verify_svn(None, None, [],
  other_prefix_path = os.path.join(sbox.wc_dir, 'prefix_other')
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  exit_code, out, err = svntest.actions.run_and_verify_svn(None, None, [],
    print("src is '%s' instead of '%s' and dest is '%s' instead of '%s'" %
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  iota_path = os.path.join(sbox.wc_dir, 'iota')
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
    None, None, [], 'diff', '-r', 'prev:head', sbox.wc_dir)
    None, None, [], 'diff', '-r', 'head:prev', sbox.wc_dir)
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
    None, None, [], 'diff', '-r', 'prev:head', sbox.wc_dir)
  "show diffs for binary files with --force"
  iota_path = os.path.join(wc_dir, 'iota')
  svntest.main.run_svn(None,
                                        expected_status, None, wc_dir)
                                        expected_status, None, wc_dir)
  # Check that we get diff when the first, the second and both files are
  # marked as binary.
  exit_code, stdout, stderr = svntest.main.run_svn(None,
                                                   'diff', '-r1:2', iota_path,
                                                   '--force')

  for line in stdout:
    if (re_nodisplay.match(line)):
      raise svntest.Failure

  exit_code, stdout, stderr = svntest.main.run_svn(None,
                                                   'diff', '-r2:1', iota_path,
                                                   '--force')

  for line in stdout:
    if (re_nodisplay.match(line)):
      raise svntest.Failure

  exit_code, stdout, stderr = svntest.main.run_svn(None,
                                                   'diff', '-r2:3', iota_path,
                                                   '--force')

  for line in stdout:
    if (re_nodisplay.match(line)):
      raise svntest.Failure
  # Check a repos->wc diff
  svntest.actions.run_and_verify_svn(None, None, [],

  add_diff = \
    make_diff_prop_header("A") + \
    make_diff_prop_added("dirprop", "r2value") + \
    make_diff_prop_header("iota") + \
    make_diff_prop_added("fileprop", "r2value")

  del_diff = \
    make_diff_prop_header("A") + \
    make_diff_prop_deleted("dirprop", "r2value") + \
    make_diff_prop_header("iota") + \
    make_diff_prop_deleted("fileprop", "r2value")


  expected_output_r1_r2 = list(make_diff_header('A', 'revision 1', 'revision 2')
                               + add_diff[:6]
                               + make_diff_header('iota', 'revision 1',
                                                   'revision 2')
                               + add_diff[7:])

  expected_output_r2_r1 = list(make_diff_header('A', 'revision 2',
                                                'revision 1')
                               + del_diff[:6]
                               + make_diff_header('iota', 'revision 2',
                                                  'revision 1')
                               + del_diff[7:])

  expected_output_r1 = list(make_diff_header('A', 'revision 1',
                                             'working copy')
                            + add_diff[:6]
                            + make_diff_header('iota', 'revision 1',
                                               'working copy')
                            + add_diff[7:])
  expected_output_base_r1 = list(make_diff_header('A', 'working copy',
                                                  'revision 1')
                                 + del_diff[:6]
                                 + make_diff_header('iota', 'working copy',
                                                    'revision 1')
                                 + del_diff[7:])
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, expected, [],
  svntest.actions.run_and_verify_svn(None, expected, [],
  svntest.actions.run_and_verify_svn(None, expected, [],
  svntest.actions.run_and_verify_svn(None, expected, [],
  svntest.actions.run_and_verify_svn(None, expected, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, expected, [],
  svntest.actions.run_and_verify_svn(None, expected, [],
                                                "working copy") + [
                                                "working copy") + [
  "@@ -1 +1,2 @@\n",
  " xxx\n",
  "+yyy\n"
  expected_output_base_r2 = make_diff_header("foo", "working copy",
  "@@ -1,2 +1 @@\n",
  " xxx\n",
  "-yyy\n"
  expected_output_r1_base = make_diff_header("foo", "revision 0",
                                                "revision 1") + [
  expected_output_base_working[3] = "+++ foo\t(working copy)\n"
  svntest.actions.run_and_verify_svn(None, [], [],
  svntest.actions.run_and_verify_svn(None, expected_output_r1_base, [],
  svntest.actions.run_and_verify_svn(None, expected_output_base_r1, [],
  svntest.actions.run_and_verify_svn(None, expected_output_r2_working, [],
  svntest.actions.run_and_verify_svn(None, expected_output_r2_base, [],
  svntest.actions.run_and_verify_svn(None, expected_output_base_r2, [],
  svntest.actions.run_and_verify_svn(None, expected_output_base_working, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, expected_output_r1_wc, [],
  svntest.actions.run_and_verify_svn(None, expected_output_wc_r1, [],
  svntest.actions.run_and_verify_svn(None, None, [],
                                     'propset', 'svn:mime-type',
  svntest.actions.run_and_verify_svn(None, expected_output_r1_wc, [],
  svntest.actions.run_and_verify_svn(None, expected_output_wc_r1, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, expected_output_r1_wc, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, expected, [],
  diff_X_r1_base = make_diff_header("X", "revision 1",
  diff_X_base_r3 = make_diff_header("X", "working copy",
  diff_foo_r1_base = make_diff_header("foo", "revision 0",
  diff_foo_base_r3 = make_diff_header("foo", "revision 0",
  diff_X_bar_r1_base = make_diff_header("X/bar", "revision 0",
  diff_X_bar_base_r3 = make_diff_header("X/bar", "revision 0",
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, expected_output_r1_base, [],
  svntest.actions.run_and_verify_svn(None, expected_output_r1_base, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, expected_output_base_r3, [],
  expected_output_r1_BASE = make_diff_header("X/bar", "revision 0",
  expected_output_r1_WORKING = make_diff_header("X/bar", "revision 0",
                                                "revision 2") + [
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, expected_output_r1_BASE, [],
  svntest.actions.run_and_verify_svn(None, expected_output_r1_WORKING, [],
  svntest.actions.run_and_verify_svn(None, None, [],
    None, svntest.verify.AnyOutput, [], 'diff', '-rBASE:1', newfile)
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, svntest.verify.AnyOutput, [],
  svntest.main.file_write(os.path.join(sbox.wc_dir, 'A', 'mu'),
                                        expected_status, None, sbox.wc_dir)
  svntest.actions.run_and_verify_svn(None,
                                     ["J. Random <jrandom@example.com>\n"],
  svntest.actions.run_and_verify_svn(None, expected_output, [],
                                        None, None, wc_dir)
  svntest.actions.run_and_verify_svn(None, [], [],
  svntest.actions.run_and_verify_svn(None, expected_output, [],
                                        None, None, wc_dir)
  svntest.actions.run_and_verify_svn(None, expected_output, [],
  C_path = os.path.join(wc_dir, "A", "C")
  D_path = os.path.join(wc_dir, "A", "D")
                                        None, None, wc_dir)
                                          None, None, wc_dir)
  svntest.actions.run_and_verify_svn(None, expected_output, [],
  diff = make_diff_prop_header(".") + \
         make_diff_prop_added("foo1", "bar1") + \
         make_diff_prop_header("iota") + \
         make_diff_prop_added("foo2", "bar2") + \
         make_diff_prop_header("A") + \
         make_diff_prop_added("foo3", "bar3") + \
         make_diff_prop_header("A/B") + \
         make_diff_prop_added("foo4", "bar4")

  dot_header = make_diff_header(".", "revision 1", "working copy")
  iota_header = make_diff_header('iota', "revision 1", "working copy")
  A_header = make_diff_header('A', "revision 1", "working copy")
  B_header = make_diff_header(B_path, "revision 1", "working copy")

  expected_empty = svntest.verify.UnorderedOutput(dot_header + diff[:7])
  expected_files = svntest.verify.UnorderedOutput(dot_header + diff[:7]
                                                  + iota_header + diff[8:14])
  expected_immediates = svntest.verify.UnorderedOutput(dot_header + diff[:7]
                                                       + iota_header
                                                       + diff[8:14]
                                                       + A_header + diff[15:21])
  expected_infinity = svntest.verify.UnorderedOutput(dot_header + diff[:7]
                                                       + iota_header
                                                       + diff[8:14]
                                                       + A_header + diff[15:21]
                                                       + B_header + diff[22:])

  os.chdir(sbox.wc_dir)

  svntest.actions.run_and_verify_svn(None, None, [],
                                     'propset',
                                     'foo1', 'bar1', '.')
  svntest.actions.run_and_verify_svn(None, None, [],
                                     'propset',
                                     'foo2', 'bar2', 'iota')
  svntest.actions.run_and_verify_svn(None, None, [],
                                     'propset',
                                     'foo3', 'bar3', 'A')
  svntest.actions.run_and_verify_svn(None, None, [],
                                     'propset',
                                     'foo4', 'bar4', os.path.join('A', 'B'))
  svntest.actions.run_and_verify_svn(None, expected_empty, [],
                                     'diff', '--depth', 'empty')
  svntest.actions.run_and_verify_svn(None, expected_files, [],
                                     'diff', '--depth', 'files')
  svntest.actions.run_and_verify_svn(None, expected_immediates, [],
                                     'diff', '--depth', 'immediates')
  svntest.actions.run_and_verify_svn(None, expected_infinity, [],
                                     'diff', '--depth', 'infinity')
  svntest.actions.run_and_verify_svn(None, None, [],
  dot_header = make_diff_header(".", "revision 1", "revision 2")
  iota_header = make_diff_header('iota', "revision 1", "revision 2")
  A_header = make_diff_header('A', "revision 1", "revision 2")
  B_header = make_diff_header(B_path, "revision 1", "revision 2")

  expected_empty = svntest.verify.UnorderedOutput(dot_header + diff[:7])
  expected_files = svntest.verify.UnorderedOutput(dot_header + diff[:7]
                                                  + iota_header + diff[8:14])
  expected_immediates = svntest.verify.UnorderedOutput(dot_header + diff[:7]
                                                       + iota_header
                                                       + diff[8:14]
                                                       + A_header + diff[15:21])
  expected_infinity = svntest.verify.UnorderedOutput(dot_header + diff[:6]
                                                       + iota_header
                                                       + diff[8:14]
                                                       + A_header + diff[15:21]
                                                       + B_header + diff[22:])

  svntest.actions.run_and_verify_svn(None, expected_empty, [],
                                     'diff', '-c2', '--depth', 'empty')
  svntest.actions.run_and_verify_svn(None, expected_files, [],
                                     'diff', '-c2', '--depth', 'files')
  svntest.actions.run_and_verify_svn(None, expected_immediates, [],
                                     'diff', '-c2', '--depth', 'immediates')
  svntest.actions.run_and_verify_svn(None, expected_infinity, [],
                                     'diff', '-c2', '--depth', 'infinity')

  diff_wc_repos = \
    make_diff_header("A/B", "revision 2", "working copy") + \
    make_diff_prop_header("A/B") + \
    make_diff_prop_modified("foo4", "bar4", "baz4") + \
    make_diff_header("A", "revision 2", "working copy") + \
    make_diff_prop_header("A") + \
    make_diff_prop_modified("foo3", "bar3", "baz3") + \
    make_diff_header("A/mu", "revision 1", "working copy") + [
    "@@ -1 +1,2 @@\n",
    " This is the file 'mu'.\n",
    "+new text\n",
    ] + make_diff_header("iota", "revision 2", "working copy") + [
    "@@ -1 +1,2 @@\n",
    " This is the file 'iota'.\n",
    "+new text\n",
    ] + make_diff_prop_header("iota") + \
    make_diff_prop_modified("foo2", "bar2", "baz2") + \
    make_diff_header(".", "revision 2", "working copy") + \
    make_diff_prop_header(".") + \
    make_diff_prop_modified("foo1", "bar1", "baz1")

  expected_empty = svntest.verify.UnorderedOutput(diff_wc_repos[49:])
  expected_files = svntest.verify.UnorderedOutput(diff_wc_repos[33:])
  expected_immediates = svntest.verify.UnorderedOutput(diff_wc_repos[13:26]
                                                       +diff_wc_repos[33:])
  expected_infinity = svntest.verify.UnorderedOutput(diff_wc_repos[:])

  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
                                     'propset',
                                     'foo1', 'baz1', '.')
  svntest.actions.run_and_verify_svn(None, None, [],
                                     'propset',
                                     'foo2', 'baz2', 'iota')
  svntest.actions.run_and_verify_svn(None, None, [],
                                     'propset',
                                     'foo3', 'baz3', 'A')
  svntest.actions.run_and_verify_svn(None, None, [],
                                     'propset',
                                     'foo4', 'baz4', os.path.join('A', 'B'))
  svntest.actions.run_and_verify_svn(None, expected_empty, [],
                                     'diff', '-rHEAD', '--depth', 'empty')
  svntest.actions.run_and_verify_svn(None, expected_files, [],
                                     'diff', '-rHEAD', '--depth', 'files')
  svntest.actions.run_and_verify_svn(None, expected_immediates, [],
                                     'diff', '-rHEAD', '--depth', 'immediates')
  svntest.actions.run_and_verify_svn(None, expected_infinity, [],
                                     'diff', '-rHEAD', '--depth', 'infinity')
                                        None, None, wc_dir)
  svntest.actions.run_and_verify_svn(None, [], [],
  diff_repos_wc = make_diff_header("A/mucopy", "revision 2", "working copy")
  svntest.actions.run_and_verify_svn(None, diff_repos_wc, [],
  svntest.main.file_append(os.path.join(wc_dir, "A", "mu"), "New mu content")
                       os.path.join(wc_dir, 'iota'))
  tau_path = os.path.join(wc_dir, "A", "D", "G", "tau")
  newfile_path = os.path.join(wc_dir, 'newfile')
  svntest.main.run_svn(None, "mkdir", os.path.join(wc_dir, 'newdir'))
                                        expected_status, None, wc_dir)
    None, None, ".*--xml' option only valid with '--summarize' option",
  # 3) Test working copy summarize
  svntest.actions.run_and_verify_diff_summarize_xml(
    ".*Summarizing diff can only compare repository to repository",
    None, wc_dir, None, None, wc_dir)

  paths = ['iota',]
  items = ['none',]
  kinds = ['file',]
  props = ['modified',]
    [], wc_dir, paths, items, props, kinds, '-c2',
    os.path.join(wc_dir, 'iota'))
  paths = ['A/mu', 'iota', 'A/D/G/tau', 'newfile', 'A/B/lambda',
           'newdir',]
  items = ['modified', 'none', 'modified', 'added', 'deleted', 'added',]
  kinds = ['file','file','file','file','file', 'dir',]
  props = ['none', 'modified', 'modified', 'none', 'none', 'none',]

  paths = ['A/mu', 'iota', 'A/D/G/tau', 'newfile', 'A/B/lambda',
           'newdir',]
  items = ['modified', 'none', 'modified', 'added', 'deleted', 'added',]
  kinds = ['file','file','file','file','file', 'dir',]
  props = ['none', 'modified', 'modified', 'none', 'none', 'none',]

  iota_path = os.path.join(sbox.wc_dir, 'iota')
  svntest.actions.run_and_verify_svn(None, [], err.INVALID_DIFF_OPTION,
  svntest.actions.run_and_verify_svn(None, expected_output, [],
@XFail()
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  make_file_edit_del_add(A);
  svntest.actions.run_and_verify_svn(None, None, [],
  make_file_edit_del_add(A2);
  svntest.actions.run_and_verify_svn(None, expected_output, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  iota_path = os.path.join(wc_dir, 'iota')
  mu_path = os.path.join(wc_dir, 'A', 'mu')
  new_path = os.path.join(wc_dir, 'new')
  lambda_path = os.path.join(wc_dir, 'A', 'B', 'lambda')
  lambda_copied_path = os.path.join(wc_dir, 'A', 'B', 'lambda_copied')
  alpha_path = os.path.join(wc_dir, 'A', 'B', 'E', 'alpha')
  alpha_copied_path = os.path.join(wc_dir, 'A', 'B', 'E', 'alpha_copied')
  expected_output = make_git_diff_header(lambda_copied_path,
                                         copyfrom_path="A/B/lambda", cp=True,
                                         "working copy",
  ] + make_git_diff_header(alpha_copied_path, "A/B/E/alpha_copied",
                         "revision 0", "working copy",
                         copyfrom_path="A/B/E/alpha", cp=True,
                         text_changes=True) + [
    " This is the file 'alpha'.\n",
    "+This is a copy of 'alpha'.\n",
  ] + make_git_diff_header(new_path, "new", "revision 0",
  ] +  make_git_diff_header(iota_path, "iota", "revision 1",
                            "working copy") + [
    "@@ -1 +1,2 @@\n",
    " This is the file 'iota'.\n",
    "+Changed 'iota'.\n",
  expected = svntest.verify.UnorderedOutput(expected_output)
  svntest.actions.run_and_verify_svn(None, expected, [], 'diff',
                                         "revision 1", "working copy",
                           "revision 1", "working copy",
                           "revision 1", "working copy",
                           copyfrom_path="A/D/G/pi", text_changes=False) \
                         copyfrom_path="A/D/G/rho", text_changes=False) \
                         copyfrom_path="A/D/G/tau", text_changes=False)
  expected = svntest.verify.UnorderedOutput(expected_output)
  svntest.actions.run_and_verify_svn(None, expected, [], 'diff',
  iota_path = os.path.join(wc_dir, 'iota')
  mu_path = os.path.join(wc_dir, 'A', 'mu')
  new_path = os.path.join(wc_dir, 'new')
  expected_output = make_git_diff_header(new_path, "new", "revision 0",
  ] + make_git_diff_header(mu_path, "A/mu", "revision 1", "working copy",
  svntest.actions.run_and_verify_svn(None, expected, [], 'diff',
  iota_path = os.path.join(wc_dir, 'iota')
  mu_path = os.path.join(wc_dir, 'A', 'mu')
  new_path = os.path.join(wc_dir, 'new')
                                         "revision 2",
    ] + make_git_diff_header("new", "new", "revision 0", "revision 2",
  svntest.actions.run_and_verify_svn(None, expected, [], 'diff',
  iota_path = os.path.join(wc_dir, 'iota')
                                        expected_status, None, wc_dir)
  svntest.actions.run_and_verify_svn(None, expected_output, [],
  iota_path = os.path.join(wc_dir, 'iota')
                                        expected_status, None, wc_dir)
  svntest.actions.run_and_verify_svn(None, expected_output, [],
  iota_path = os.path.join(wc_dir, 'iota')
  new_path = os.path.join(wc_dir, 'new')
                                        expected_status, None, wc_dir)
  expected_output = make_git_diff_header(new_path, "new", "revision 0",
  svntest.actions.run_and_verify_svn(None, expected_output, [], 'diff',
  iota_path = os.path.join(wc_dir, 'iota')
  new_path = os.path.join(wc_dir, 'new')
                                        expected_status, None, wc_dir)
                                         "revision 0", "working copy",
                                         "revision 1", "working copy",
  svntest.actions.run_and_verify_svn(None, expected_output, [], 'diff',
  svntest.main.run_svn(None, 'ps', 'a','b', wc_dir)
                                        expected_status, None, wc_dir)
  expected_output = make_git_diff_header(".", "", "revision 1",
                    make_diff_prop_added("a", "b")
  svntest.actions.run_and_verify_svn(None, expected_output, [], 'diff',
  A_path = os.path.join(wc_dir, 'A')
  B_abs_path = os.path.abspath(os.path.join(wc_dir, 'A', 'B'))
  svntest.actions.run_and_verify_svn(None, None, [], 'diff', B_abs_path)
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  svntest.actions.run_and_verify_svn(None, None, [],
  expected_output = make_diff_header("chi", "revision 1", "revision 2") + [
                                         "revision 2") + [
                                         "revision 2") + [
  svntest.actions.run_and_verify_svn(None, expected_output, [],
  svntest.actions.run_and_verify_svn(None, expected_output, [],
  # skip actually testing the output since apparently 1.7 is busted
  # this isn't related to issue #4460, older versions of 1.7 had the issue
  # as well
  #expected_output = make_diff_header(iota, 'working copy', 'revision 1') + \
  #                  [ '@@ -1,2 +1 @@\n',
  #                    " This is the file 'iota'.\n",
  #                    "-second line\n", ] + \
  #                  make_diff_prop_header(iota) + \
  #                  make_diff_prop_deleted('svn:mime-type', 'text/plain')
  expected_output = None
  svntest.actions.run_and_verify_svn(None, expected_output, [],
  svntest.actions.run_and_verify_svn(None, None, [], 'diff',
  svntest.actions.run_and_verify_svn(None, None, [], 'diff',
    # remove line with the same content.  Fortunately, it does't
  svntest.actions.run_and_verify_svn(None, expected_output, [],
    expected_output = make_diff_header(newfile, 'revision 1', 'revision 2') + \
    svntest.actions.run_and_verify_svn(None, expected_output, [], 'diff',
    # Note no property delete is printed when whole file is deleted
    expected_output = make_diff_header(newfile, 'revision 2', 'revision 1') + \
                      [ '@@ -1, +0,0 @@\n',
                        "-This is the file 'newfile'.\n" ]
    svntest.actions.run_and_verify_svn(None, None, [], 'diff',