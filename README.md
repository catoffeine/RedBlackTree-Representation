____
# Description
That is a program, that will display a red-black tree with the possibility to delete/add nodes
____
## Demonstration
![OnOpen](https://github.com/mikhailku245/RedBlackTree-Representation/raw/master/imagesForReadme/RBTreeOnOpen.PNG)
____
DoubleR/HalfR -> buttons for doubling and halfing radius of the nodes    
DoubleX/HalfX -> buttons for doubling and halfing distance between sibling nodes
____
## How to start tests?
1) Input a `number` or a `range` in the text edit field under the DoubleX/HalfX buttons via ',' or ';' (ex. `150,250`)
(If you enter just a number, then the range of generating values will be: {0, `yourNumber`})
2) Enter count of tests in the field to the right of the button `Start Test`
3) Push button `Start Test`, the numbers will generate in the current range with chance, which depends on count of nodes (maximum - 40, if the number of elements will approach to this value, the chance of deleting an element will increase accordingly, that's an optimal count)
![Tests](https://github.com/mikhailku245/RedBlackTree-Representation/raw/master/imagesForReadme/RBTreeTests.png)
## How to save tree?
When you push button `Save Tree`, your current State increases by 1    
If you want to return to this state, simply click on the `Back State` button or `Forward State` to return back again    
(Note: before returning back, you must save tree again, or it just will delete the state with which you came)
