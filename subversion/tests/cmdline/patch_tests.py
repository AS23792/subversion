  mu_path = os.path.join(wc_dir, 'A', 'mu')
                                        expected_status, None, wc_dir)
    'U         %s\n' % os.path.join(wc_dir, 'A', 'D', 'gamma'),
    'U         %s\n' % os.path.join(wc_dir, 'iota'),
    'A         %s\n' % os.path.join(wc_dir, 'new'),
    'U         %s\n' % os.path.join(wc_dir, 'A', 'mu'),
    'D         %s\n' % os.path.join(wc_dir, 'A', 'B', 'E', 'beta'),
  sbox.build()
    'Summary of conflicts:\n',
    '  Skipped paths: 1\n'
  ]
  expected_status = svntest.actions.get_virginal_state('.', 1)
    lambda_path:  Item(),
  svntest.actions.run_and_verify_patch('.', os.path.abspath(patch_file_path),
  mu_path = os.path.join(wc_dir, 'A', 'mu')
  iota_path = os.path.join(wc_dir, 'iota')
                                        expected_status, None, wc_dir)
  expected_status = svntest.actions.get_virginal_state('.', 1)
  svntest.actions.run_and_verify_patch('.', os.path.abspath(patch_file_path),
  mu_path = os.path.join(wc_dir, 'A', 'mu')
                                        expected_status, None, wc_dir)
    'U         %s\n' % os.path.join(wc_dir, 'A', 'D', 'gamma'),
    'U         %s\n' % os.path.join(wc_dir, 'iota'),
    'A         %s\n' % os.path.join(wc_dir, 'new'),
    'U         %s\n' % os.path.join(wc_dir, 'A', 'mu'),
    'D         %s\n' % os.path.join(wc_dir, 'A', 'B', 'E', 'beta'),
  mu_path = os.path.join(wc_dir, 'A', 'mu')
                                        expected_status, None, wc_dir)
    'U         %s\n' % os.path.join(wc_dir, 'A', 'D', 'gamma'),
    'U         %s\n' % os.path.join(wc_dir, 'iota'),
    'A         %s\n' % os.path.join(wc_dir, 'new'),
    'U         %s\n' % os.path.join(wc_dir, 'A', 'mu'),
    'D         %s\n' % os.path.join(wc_dir, 'A', 'B', 'E', 'beta'),
  gamma_path = os.path.join(wc_dir, 'A', 'D', 'gamma')
  iota_path = os.path.join(wc_dir, 'iota')
                                        expected_status, None, wc_dir)
    'U         %s\n' % os.path.join(wc_dir, 'A', 'D', 'gamma'),
    'U         %s\n' % os.path.join(wc_dir, 'iota'),
  sbox.build()
  C_path = os.path.join(wc_dir, 'A', 'C')
  E_path = os.path.join(wc_dir, 'A', 'B', 'E')
  svntest.actions.run_and_verify_svn("Deleting C failed", None, [],
                                     'rm', C_path)
  svntest.actions.run_and_verify_svn("Deleting E failed", None, [],
                                     'rm', E_path)
  A_B_E_Y_new_path = os.path.join(wc_dir, 'A', 'B', 'E', 'Y', 'new')
  A_C_new_path = os.path.join(wc_dir, 'A', 'C', 'new')
  A_Z_new_path = os.path.join(wc_dir, 'A', 'Z', 'new')
    'A         %s\n' % os.path.join(wc_dir, 'X'),
    'A         %s\n' % os.path.join(wc_dir, 'X', 'Y'),
    'A         %s\n' % os.path.join(wc_dir, 'X', 'Y', 'new'),
    'Summary of conflicts:\n',
    '  Skipped paths: 3\n',
  ]
  expected_disk.remove('A/B/E/alpha')
  expected_disk.remove('A/B/E/beta')
  if svntest.main.wc_is_singledb(wc_dir):
    expected_disk.remove('A/B/E')
    expected_disk.remove('A/C')
           'X'         : Item(status='A ', wc_rev=0),
           'X/Y'       : Item(status='A ', wc_rev=0),
           'X/Y/new'   : Item(status='A ', wc_rev=0),
           'A/B/E'     : Item(status='D ', wc_rev=1),
           'A/B/E/alpha': Item(status='D ', wc_rev=1),
           'A/B/E/beta': Item(status='D ', wc_rev=1),
           'A/C'       : Item(status='D ', wc_rev=1),
  expected_skip = wc.State('', {A_Z_new_path : Item(),
                                A_B_E_Y_new_path : Item(),
                                A_C_new_path : Item()})
  sbox.build()
  F_path = os.path.join(wc_dir, 'A', 'B', 'F')
  svntest.actions.run_and_verify_svn("Deleting F failed", None, [],
  svntest.actions.run_and_verify_svn("Update failed", None, [],
  os.remove(os.path.join(wc_dir, 'A', 'D', 'H', 'chi'))
    'D         %s\n' % os.path.join(wc_dir, 'A', 'D', 'H', 'psi'),
    'D         %s\n' % os.path.join(wc_dir, 'A', 'D', 'H', 'omega'),
    'D         %s\n' % os.path.join(wc_dir, 'A', 'B', 'lambda'),
    'D         %s\n' % os.path.join(wc_dir, 'A', 'B', 'E', 'alpha'),
    'D         %s\n' % os.path.join(wc_dir, 'A', 'B', 'E', 'beta'),
    'D         %s\n' % os.path.join(wc_dir, 'A', 'B'),
  expected_disk.remove('A/D/H/chi')
  expected_disk.remove('A/D/H/psi')
  expected_disk.remove('A/D/H/omega')
  expected_disk.remove('A/B/lambda')
  expected_disk.remove('A/B/E/alpha')
  expected_disk.remove('A/B/E/beta')
  if svntest.main.wc_is_singledb(wc_dir):
    expected_disk.remove('A/B/E')
    expected_disk.remove('A/B/F')
    expected_disk.remove('A/B')
  gamma_path = os.path.join(wc_dir, 'A', 'D', 'gamma')
                                        expected_status, None, wc_dir)
    'C         %s\n' % os.path.join(wc_dir, 'A', 'D', 'gamma'),
    'Summary of conflicts:\n',
    '  Text conflicts: 1\n',
  ]
  gamma_path = os.path.join(wc_dir, 'A', 'D', 'gamma')
                       os.path.join(wc_dir, 'A', 'D', 'gamma'))
                                        expected_status, None, wc_dir)
    'U         %s\n' % os.path.join(wc_dir, 'A', 'D', 'gamma'),
  mu_path = os.path.join(wc_dir, 'A', 'mu')
                                      expected_status, None, wc_dir)
    'U         %s\n' % os.path.join(wc_dir, 'A', 'mu'),
  mu_path = os.path.join(wc_dir, 'A', 'mu')
                                        expected_status, None, wc_dir)
    'U         %s\n' % os.path.join(wc_dir, 'A', 'D', 'gamma'),
    'U         %s\n' % os.path.join(wc_dir, 'iota'),
    'A         %s\n' % os.path.join(wc_dir, 'new'),
    'U         %s\n' % os.path.join(wc_dir, 'A', 'mu'),
    'D         %s\n' % os.path.join(wc_dir, 'A', 'B', 'E', 'beta'),
  sbox.build()
  mu_path = os.path.join(wc_dir, 'A', 'mu')

        'G         %s\n' % os.path.join(wc_dir, 'A', 'mu'),
      svntest.actions.run_and_verify_svn(None, expected_output, [], 'revert', '-R', wc_dir)
  mu_path = os.path.join(wc_dir, 'A', 'mu')

        'U         %s\n' % os.path.join(wc_dir, 'A', 'mu'),
      svntest.actions.run_and_verify_svn(None, expected_output, [], 'revert', '-R', wc_dir)
  sbox.build()
  mu_path = os.path.join(wc_dir, 'A', 'mu')

        'G         %s\n' % os.path.join(wc_dir, 'A', 'mu'),
      svntest.actions.run_and_verify_svn(None, expected_output, [], 'revert', '-R', wc_dir)
  mu_path = os.path.join(wc_dir, 'A', 'mu')
                                        expected_status, None, wc_dir)
    'U         %s\n' % os.path.join(wc_dir, 'A', 'mu'),
  mu_path = os.path.join(wc_dir, 'A', 'mu')
                                        expected_status, None, wc_dir)
  iota_path = os.path.join(wc_dir, 'iota')
                                        expected_status, None, wc_dir)
    'U         %s\n' % os.path.join(wc_dir, 'iota'),
  iota_path = os.path.join(wc_dir, 'iota')
                                        expected_status, None, wc_dir)
    ' U        %s\n' % os.path.join(wc_dir, 'iota'),
  mu_path = os.path.join(wc_dir, 'A', 'mu')
  beta_path = os.path.join(wc_dir, 'A', 'B', 'E', 'beta')
                                        expected_status, None, wc_dir)
    'U         %s\n' % os.path.join(wc_dir, 'A', 'D', 'gamma'),
    'U         %s\n' % os.path.join(wc_dir, 'iota'),
    'A         %s\n' % os.path.join(wc_dir, 'new'),
    'U         %s\n' % os.path.join(wc_dir, 'A', 'mu'),
    'G         %s\n' % os.path.join(wc_dir, 'A', 'D', 'gamma'),
    'G         %s\n' % os.path.join(wc_dir, 'iota'),
    'G         %s\n' % os.path.join(wc_dir, 'new'),
    'G         %s\n' % os.path.join(wc_dir, 'A', 'mu'),
    'Summary of conflicts:\n',
    '  Skipped paths: 1\n',
  ]
  expected_skip = wc.State('', {beta_path : Item()})
  B_path = os.path.join(wc_dir, 'A', 'B')
                                        expected_status, None, wc_dir)
    ' C        %s\n' % os.path.join(wc_dir, 'A', 'B'),
    'Summary of conflicts:\n',
    '  Property conflicts: 1\n',
  ]
  sbox.build()
  iota_path = os.path.join(wc_dir, 'iota')
    'A         %s\n' % os.path.join(wc_dir, 'new'),
    'A         %s\n' % os.path.join(wc_dir, 'X'),
  iota_path = os.path.join(wc_dir, 'iota')
                                        expected_status, None, wc_dir)
  expected_status = svntest.actions.get_virginal_state('.', 1)
  svntest.actions.run_and_verify_patch('.', os.path.abspath(patch_file_path),
  mu_path = os.path.join(wc_dir, 'A', 'mu')
                                      expected_status, None, wc_dir)
    ' U        %s\n' % os.path.join(wc_dir, 'A', 'mu'),
  sbox.build()
  new_path = os.path.join(wc_dir, 'new')
    'A         %s\n' % os.path.join(wc_dir, 'new'),
    'D         %s\n' % os.path.join(wc_dir, 'iota'),
  mu_path = os.path.join(wc_dir, 'A', 'mu')
                                        expected_status, None, wc_dir)
    'U         %s\n' % os.path.join(wc_dir, 'A', 'mu'),
  mu_path = os.path.join(wc_dir, 'A', 'mu')
                                        expected_status, None, wc_dir)
    'U         %s\n' % os.path.join(wc_dir, 'A', 'D', 'gamma'),
    'U         %s\n' % os.path.join(wc_dir, 'iota'),
    'A         %s\n' % os.path.join(wc_dir, 'new'),
    'U         %s\n' % os.path.join(wc_dir, 'A', 'mu'),
    'D         %s\n' % os.path.join(wc_dir, 'A', 'B', 'E', 'beta'),
    'G         %s\n' % os.path.join(wc_dir, 'A', 'D', 'gamma'),
    'G         %s\n' % os.path.join(wc_dir, 'iota'),
    'D         %s\n' % os.path.join(wc_dir, 'new'),
    'G         %s\n' % os.path.join(wc_dir, 'A', 'mu'),
    'A         %s\n' % os.path.join(wc_dir, 'A', 'B', 'E', 'beta'),
  sbox.build()
  mu_path = os.path.join(wc_dir, 'A', 'mu')
  sbox.build()
    'A         %s\n' % os.path.join(wc_dir, 'iota_symlink'),
  iota_path = os.path.join(wc_dir, 'iota')
                                        expected_status, None, wc_dir)
    ' U        %s\n' % os.path.join(wc_dir, 'iota'),
  # Apply patch 
                                       0, # dry-run
                                       '--reverse-diff') 
  sbox.build()
  newfile_path = os.path.join(wc_dir, 'newfile')
                                       0, # dry-run
                                       '--reverse-diff') 
  newfile_path = os.path.join(wc_dir, 'newfile')
                                        expected_status, None, wc_dir)
                                       0, # dry-run
                                       '--reverse-diff') 
  sbox.build()
    'A         %s\n' % os.path.join(wc_dir, 'new'),
    'D         %s\n' % os.path.join(wc_dir, 'A', 'B', 'E', 'beta'),
  sbox.build()
    'D         %s\n' % os.path.join('A', 'B', 'E'),
    'Summary of conflicts:\n',
    '  Skipped paths: 1\n'
  ]
  expected_status = svntest.actions.get_virginal_state('.', 1)
  expected_skip = wc.State('', {skipped_path: Item()})
  svntest.actions.run_and_verify_patch('.', os.path.abspath(patch_file_path),
@Issue(3991)
def patch_lacking_trailing_eol(sbox):
  "patch file lacking trailing eol"
  sbox.build(read_only = True)
  iota_path = os.path.join(wc_dir, 'iota')
  mu_path = os.path.join(wc_dir, 'A', 'mu')

  # Prepare
  expected_status = svntest.actions.get_virginal_state(wc_dir, 1)

  # Apply patch
  unidiff_patch = [
    "Index: iota\n",
    "===================================================================\n",
    "--- iota\t(revision 1)\n",
    "+++ iota\t(working copy)\n",
    # TODO: -1 +1
    "@@ -1 +1,2 @@\n",
    " This is the file 'iota'.\n",
    "+Some more bytes", # No trailing \n on this line!
  ]

  svntest.main.file_write(patch_file_path, ''.join(unidiff_patch))

  gamma_contents = "It is the file 'gamma'.\n"
  iota_contents = "This is the file 'iota'.\n"
  new_contents = "new\n"

  expected_output = [
    'U         %s\n' % os.path.join(wc_dir, 'iota'),
  ]

  # Expect a newline to be appended
  expected_disk = svntest.main.greek_state.copy()
  expected_disk.tweak('iota', contents=iota_contents + "Some more bytes")

  expected_status = svntest.actions.get_virginal_state(wc_dir, 1)
  expected_status.tweak('iota', status='M ')

  expected_skip = wc.State('', { })

  svntest.actions.run_and_verify_patch(wc_dir, os.path.abspath(patch_file_path),
                                       expected_output,
                                       expected_disk,
                                       expected_status,
                                       expected_skip,
                                       None, # expected err
                                       1, # check-props
                                       1) # dry-run

def patch_target_no_eol_at_eof(sbox):
  "patch target with no eol at eof"

  sbox.build()
  wc_dir = sbox.wc_dir

  patch_file_path = make_patch_path(sbox)
  iota_path = os.path.join(wc_dir, 'iota')
  mu_path = sbox.ospath('A/mu')
                                        expected_status, None, wc_dir)
    'U         %s\n' % os.path.join(wc_dir, 'A', 'mu'),
    'U         %s\n' % os.path.join(wc_dir, 'iota'),
    'A         %s\n' % os.path.join(wc_dir, 'P'),
    'A         %s\n' % os.path.join(wc_dir, 'P', 'Q'),
    'A         %s\n' % os.path.join(wc_dir, 'P', 'Q', 'foo'),
    'D         %s\n' % os.path.join(wc_dir, 'iota'),
      'src/'                            : Item(status='A ', wc_rev=0),
      'src/tools/ConsoleRunner/'        : Item(status='A ', wc_rev=0),
              patch_lacking_trailing_eol,