import difflib

def diff_files(p1, p2):
    with open(p1) as f1, open(p2) as f2:
        l1 = f1.readlines()
        l2 = f2.readlines()
    diff = list(difflib.unified_diff(l1, l2, fromfile=p1, tofile=p2))
    return ''.join(diff[:50])

print("--- d_pausewindow diff ---")
print(diff_files('scratch/orig_pausewindow.txt', 'scratch/test_pausewindow.txt'))

print("--- d_controller_info diff ---")
print(diff_files('scratch/orig_ctrl.txt', 'scratch/test_controller_info.txt'))

print("--- d_yes_no diff ---")
print(diff_files('scratch/orig_yn.txt', 'scratch/test_yes_no.txt'))
