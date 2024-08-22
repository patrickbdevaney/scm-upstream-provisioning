import os
import subprocess

def git_add_commit_push(repo_path, batch_size=100):
    os.chdir(repo_path)
    files = []
    for root, _, filenames in os.walk(repo_path):
        for filename in filenames:
            files.append(os.path.join(root, filename))

    total_files = len(files)
    batch_number = 1

    for i in range(0, total_files, batch_size):
        batch_files = files[i:i + batch_size]
        for file in batch_files:
            subprocess.run(['git', 'add', file])
        commit_message = f"commit {batch_number}"
        subprocess.run(['git', 'commit', '-m', commit_message])
        subprocess.run(['git', 'push'])
        batch_number += 1

    # remaining files
    remaining_files = total_files % batch_size
    if remaining_files > 0:
        batch_files = files[-remaining_files:]
        for file in batch_files:
            subprocess.run(['git', 'add', file])
        subprocess.run(['git', 'commit', '-m', 'commit'])
        subprocess.run(['git', 'push'])

#define the absolute path of the repo here
repo_path = 'C:\\'
git_add_commit_push(repo_path)
