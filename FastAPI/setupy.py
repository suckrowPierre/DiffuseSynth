from setuptools import setup, find_packages

setup(
    name='server',
    version='0.1',
    packages=find_packages(),
    install_requires=[
        'torch'
        'git+https://github.com/huggingface/diffusers.git'
        'git+https://github.com/huggingface/transformers.git'
        'transformers'
        'fastapi'
        'uvicorn'
        'pydantic'
        'accelerate'
        'soundfile'
    ],
    entry_points={
        'console_scripts': [
            'server=server:main',  # Replace `myapp:main` with `{your_python_filename_without_extension}:{entry_point_function}`
        ],
    }
)
