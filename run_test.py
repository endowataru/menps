
import sys
import subprocess

program_name = sys.argv[1]
try:
    process_count = sys.argv[2]
except IndexError:
    process_count = 1

def extract_testcases(gtest_output):
    result = []
    test_case = ""
    for l in gtest_output.split("\n"):
        l = l.strip()
        if len(l) == 0:
            continue
        
        if "." in l:
            test_case = l.replace(".", "")
        else:
            test_name = l
            result.append(test_case + "." + test_name)
    
    return result

process = subprocess.Popen([program_name, "--gtest_list_tests"], stdout=subprocess.PIPE)
out, _ = process.communicate()

#print(out)

#print(extract_testcases(out))

for testcase in extract_testcases(out):
    print(["mpirun", "-n", str(process_count), program_name, "--gtest_filter="+testcase])
    subprocess.check_call(["mpirun", "-n", str(process_count), program_name, "--gtest_filter="+testcase])


