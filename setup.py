from setuptools import setup, find_packages

setup(
        name = 'fuzzle',
        version = '0.0.6',
        description = 'a tool set and framework to enhance emedded and unique fuzzing',
        author = 'JeffJerseyCow',
        author_email = 'jeffjerseycow@gmail.com',
        url = 'https://github.com/JeffJerseyCow/fuzzle',
        packages = find_packages(),
        install_requires = ['pygdbmi>=0.8.2.0'],
        entry_points = {'console_scripts':['duzzle = fuzzle.duzzle.__main__:main']},
)
