---
layout: post
title:  "Build System Craftsmanship"
date:   2015-09-14 00:00:00
categories: craftsmanship
---

Craftsmanship seems to be on everyones mind recently. A great resources that gives pragmatic options on being a craftsman is _Pragmatic Programmer_, one of my favorite topics that they discuss is __Broken Windows__.

> Don't leave "broken windows" (bad designs, wrong decisions, or poor code) unrepaired. Fix each one as soon as it is discovered. If there is insufficient time to fix it properly, then board it up. Perhaps you can comment out the offending code, or display a "Not Implemented" message, or substitute dummy data instead. Take some action to prevent further damage and to show that you're on top of the situation. 
> We've seen clean, functional systems deteriorate pretty quickly once windows start breaking. There are other factors that can contribute to software rot, and we'll touch on some of them elsewhere, but neglect accelerates the rot faster than any other factor.
> — __Pragmatic Programmer__

A craftsman, strives to fix [Broken Windows](http://blog.codinghorror.com/the-broken-window-theory/). They know the result of every broken window and how they can infect entire codebases. Fixing these issues, as best as time and resources allow, raises the quality across the board and gives others great examples to follow. We can’t always fix everything wrong with a project, but if we follow the [Boy Scout Rule](http://programmer.97things.oreilly.com/wiki/index.php/The_Boy_Scout_Rule), the windows will be fixed over time.

When analyzing the quality of a given codebase, we have high standards for the production source, but often low/no standards for the build source([_91 Things Every Developer Should Know: Own (and Refactor) the Build_](http://programmer.97things.oreilly.com/wiki/index.php/Own_%28and_Refactor%29_the_Build)). This is because build source don’t form windows, they are the foundation. You can patch the foundation as much as you want, but it will cost more the longer it’s broken. Broken foundations cause more damage than broken windows because it leaks into everything, slowing everyone down when there is an issue and keeping a few busy unable to break free of the constant fix -\> regression -\> patch cycle.

The only way to break the cycle is to treat your build sources as production sources. That means following all the processes that any other change would require. This means writing unit tests, having a good design, and having code reviews. When the nexus of good software practices join, you start to prevent regressions. Using Gradle helps us do this. Gradle doesn't solve the problem, but gives engineers the tools to solve it.
