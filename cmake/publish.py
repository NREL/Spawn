import subprocess
import glob
import requests
import json
import os
import shutil
import re
import sys

package_path = glob.glob('build/Spawn-[0-9].[0-9].[0-9]*')[0]
package_name = os.path.basename(package_path)

# Copy to AWS S3 bucket
# Copy package to a new location omitting the version info
# This is to provide a stable download location
latest_package_name = re.sub(r'[0-9]\.[0-9]\.[0-9]-[a-zA-Z_0-9]*','latest',package_name)
latest_package_path = 'build/' + latest_package_name
shutil.copyfile(package_path, latest_package_path)
subprocess.run(['aws', 's3', 'cp', '--acl', 'public-read', latest_package_path, 's3://spawn/latest/'])
subprocess.run(['aws', 's3', 'cp', '--acl', 'public-read', package_path, 's3://spawn/builds/'])

if sys.argv[1] == 'release':
    project_id = '10361647'
    release_id = os.environ['CI_COMMIT_TAG']
    headers = {"PRIVATE-TOKEN": os.environ['GITLAB_TOKEN']}
    
    # Create a release (it may already exists, but this is just in case)
    gitlab_api_url = "https://gitlab.com/api/v4/projects/%s/releases" % (project_id)
    payload = {'name': release_id, 'tag_name': release_id, 'description': release_id + ' Release'}
    r = requests.post(gitlab_api_url, headers=headers, data=payload)
    print('Create release response: ' + r.text)
    
    # Add a link to the installer package
    s3_url = 'https://spawn.s3.amazonaws.com/builds/' + package_name
    payload = {'name': package_name, 'url': s3_url}
    
    gitlab_api_url = "https://gitlab.com/api/v4/projects/%s/releases/%s/assets/links" % (project_id, release_id)
    #r = requests.post(gitlab_api_url, data=payload)
    r = requests.post(gitlab_api_url, headers=headers, data=payload)
    print('Link asset response: ' + r.text)

