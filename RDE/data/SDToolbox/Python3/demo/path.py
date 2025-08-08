"""
Reaction path diagram example with some options
Author: Tilman Bremer
Date: June 29th 2017

See Tilman's blog post
https://www.tilmanbremer.de/2017/06/tutorial-generating-reaction-path-diagrams-with-cantera-and-python/
 
Written for Python 3.6 with Cantera 2.3.0 but might as well work with other versions.

Modifications by JES 11 May 2019 
"""
import os
import cantera as ct
 
# Define a gas mixture at a high temperature that will undergo a reaction:
mech = 'Hong2011.cti'
gas  = ct.Solution(mech)
gas.TPX = 933.94, 99662326.7, 'H2:2,O2:1'
 
# Define a constant volume reactor, let it react until the temperature reaches 1800 K:
r = ct.IdealGasReactor(gas)   
net = ct.ReactorNet([r])
T = r.T
while T < 950.0:
    net.step()
    T = r.T

print("time {:10.3e}".format(net.time))
print("temperature {:10.3e}".format(r.T))
# Define the element to follow in the reaction path diagram:
element = 'H'
 
# Initiate the reaction path diagram:
diagram = ct.ReactionPathDiagram(gas, element)
 
# Options for cantera:
diagram.show_details = True
#diagram.show_details = False
diagram.font='CMU Serif Roman'
#diagram.font='Arial'
diagram.threshold=0.01
bold_color = 1
diagram.dot_options='nodesep=2;ratio="compress";size = "7.5,4.6"; node[fontsize=12,shape="box"]'
#diagram.title = 'Reaction path diagram following {0}'.format(element)
#diagram.arrow_width=2.0
 
# Define the filenames:
dot_file = 'ReactionPathDiagram.dot'
img_file = 'ReactionPathDiagram.pdf'
modified_dot_file = 'ReactionPathDiagramModified.dot'
 
# Write the dot-file first, then create the image from the dot-file with customizable
# parameters:
diagram.write_dot(dot_file)

# modify dot file to change line color, style 
# Open the  dot file and adjustments before generating the image
# The dot_file is opened and read, the adjustements are saved in the modified_dot_file:
with open(modified_dot_file, 'wt') as outfile:
  with open(dot_file, 'rt') as infile:
    for row in infile:
      # Remove the line with the label:
      if row.startswith(' label'):
        row = ""
      # Replace the color statements:
      row = row.replace('color="0.7', 'color="1')
      row = row.replace(', 0.9"', ', 0.0"')
      row = row.replace('style="setlinewidth(', 'penwidth=')
      row = row.replace(')", arrowsize', ', arrowsize')
      outfile.write(row)
      
# The last command requires dot to be in your system path variables, or your system
# will not undersand the command "dot".
# The command -Tpng defines the filetype and needs to fit your filename defined above,
# or else you will get errors opening the file later.
# The command -Gdpi sets the resolution of the generated image in dpi.
##os.system('dot {0}  -Gdpi=300 -Tpng -o{1}'.format(dot_file, img_file))
os.system('dot {0}  -Tpdf -o{1}'.format(modified_dot_file, img_file))
# compute creation and destruction rates for each species
print('Number of species '+str(gas.n_species))
print('Number of reactions '+str(gas.n_reactions))
moles = 0.0
print('Creation, destruction,  net rates and concentrations for each species')
for i, value in enumerate(gas.species_names):
    print("{:3d}".format(i+1), "{:5s}".format(value), "{:9.2e}".format(gas.creation_rates[i]),"{:9.2e}".format(gas.destruction_rates[i]),"{:9.2e}".format(gas.net_production_rates[i]),"{:9.2e}".format(gas.concentrations[i]))
    moles = moles + gas.concentrations[i]
print("Total molar concentration {:9.2e}".format(moles))
print('Forward, Reverse and net rates of progress for each reaction')
for i, value in enumerate(gas.reaction_equations()):
    print("{:3d}".format(i+1), "{:30s}".format(value),"{:9.2e}".format(gas.forward_rates_of_progress[i]),"{:9.2e}".format(gas.reverse_rates_of_progress[i]),"{:9.2e}".format(gas.net_rates_of_progress[i]))
print('Forward and Reverse rate constants for each reaction')
for i, value in enumerate(gas.reaction_equations()):
    print("{:3d}".format(i+1), "{:30s}".format(value), "{:9.2e}".format(gas.forward_rate_constants[i]),"{:9.2e}".format(gas.reverse_rate_constants[i]))
