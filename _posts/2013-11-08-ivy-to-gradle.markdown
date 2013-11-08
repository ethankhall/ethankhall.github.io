---
layout: post
title:  "Ivy to Grady"
date:   2013-11-08 07:10:00
categories: gradle
---

I have been messing around with a project that uses Ant and Ivy as it's build tools. I find this unacceptable since they are both aweful to work with, and make it hard for me to do local changes without posting to a repository. I am working on switching the project to Gradle. Here is a script that you may find useful should you want to do the same. It will take an ivy.xml file and convert it into a gradle dependencies closure. It doesn't do anything fancy, but it gets rid of some sed operations.

{% gist 7370807 ivyConverter.groovy %}
