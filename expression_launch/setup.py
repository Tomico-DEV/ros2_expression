from setuptools import find_packages, setup
from glob import glob

package_name = 'expression_launch'

setup(
    name=package_name,
    version='0.0.1',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        ('share/' + package_name + '/launch', glob('launch/*')),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='tomicodev',
    maintainer_email='humansbadrobotsgood@gmail.com',
    description='Package for conveniently launching the Expression stack',
    license='Apache-2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
          f'lifecycle_manager = {package_name}.lifecycle_manager:main'
        ],
    },
)