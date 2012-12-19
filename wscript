#! /usr/bin/env python

import glob

APPNAME = 'nbody'
VERSION = '1.0'

top = '.'
out = 'build'

def options(ctx):
	ctx.load('compiler_cxx compiler_c')
	ctx.add_option(
		'-m', '--mode',
		action			= 'store',
		default			= 'debug',
		help			= 'build mode, release or debug'
	)

def configure(ctx):
	ctx.load('compiler_cxx compiler_c')

	# OSG debug
	ctx.check(
		uselib_store    = 'OSGd',
		lib             = ['osgd', 'OpenThreadsd', 'osgDBd', 'osgUtild', 'osgViewerd', 'osgSimd'],
		framework       = ['Cocoa', 'OpenGL', 'IOKit'],
		libpath 		= '/usr/local/lib'
	)

def build(ctx):
	cxxflags = {
		'release': ['-O4', '-w', '-DNDEBUG'],
		'debug': ['-O0', '-W', '-Wall', '-g']
	}
	ctx.env.append_unique('CXXFLAGS', cxxflags[ctx.options.mode])
	#ctx.env.append_unique('CXXFLAGS', '-std=c++11')

	ctx.program(
		target			= 'planets',
		use 			= ['OSGd'],
		includes 		= 'externals',
		source			= ctx.path.ant_glob([
								'source/*.cc',
								'source/**/*.cc',
								'source/*.c',
								'source/**/*.c'
							])
	)

def dist(ctx):
	ctx.files = ctx.path.ant_glob([
						'wscript', 'waf', 'LISEZ-MOI', \
						'source/**.cc', 'source/**.hxx', 'source/**.hh',\
						'externals/**/*.cc', 'externals/**/*.hxx', 'externals/**/*.hh',\
						'runtime/*'
					])