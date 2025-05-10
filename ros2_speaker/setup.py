from setuptools import find_packages, setup

package_name = 'ros2_speaker'

setup(
    name=package_name,
    version='0.0.1',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='tomicodev',
    maintainer_email='humansbadrobotsgood@gmail.com',
    description='ros2 package for text-to-speech',
    license='Apache-2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'voice_server = ros2_speaker.server:main'
        ],
    },
)
