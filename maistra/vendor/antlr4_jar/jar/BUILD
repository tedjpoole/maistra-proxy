load("@rules_java//java:defs.bzl", "java_import")

package(default_visibility = ["//visibility:public"])

java_import(
  name = 'jar',
  jars = ["downloaded.jar"],
  visibility = ['//visibility:public'],
)

filegroup(
  name = 'file',
  srcs = ["downloaded.jar"],
  visibility = ['//visibility:public'],
)

