package boids.viewbuilder.test;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.HashMap;

import boids.viewbuilder.Boid;
import boids.viewbuilder.BoidModel;
import boids.viewbuilder.OverlayBoids;

/** ----------------------------< Boid Model View Builder >----------------------------
 *	Defines the testbed for the boid model.
 *	
 *	@author Victor Colton
 */
public class BoidModelTest 
{
	HashMap<String, Double> parameters = new HashMap<String, Double>();
	BoidModel _model;
	
	public static void main(final String[] arguments)
	{
		BoidModelTest tester = new BoidModelTest();
		
		tester.test(arguments);
	}

	private void test(String[] arguments) 
	{
		String outputPath = "C:\\Users\\csfaculty\\Dropbox\\MPI\\data\\";
		String inputPath = "C:\\Users\\csfaculty\\Dropbox\\MPI\\data\\";
		String inputFile = arguments.length>0?arguments[0]:"boid_data.boid";
		String outputFile = arguments.length>1?arguments[1]:"boids.gif";
		String gnuscriptFile = arguments.length>2?arguments[2]:"boid_script.gp";
		String[] params;
		String delim = ";";
		String input;
		BufferedReader in = null;
		Boid newBoid;
		int delay = 5;
		int p, b;
		
		// Open the input file to read
		try 
		{
			 in = new BufferedReader(new FileReader(inputPath+inputFile));
		} 
		catch (FileNotFoundException e) 
		{
			System.err.println("Cannot open input file: "+inputPath+inputFile+"; "+e);
		}
		
		// Read parameters from the input file header
		try 
		{
			input = in.readLine();
			while ( input.compareTo("#header") != 0 )
			{
				input = in.readLine();
			}
			
			input = in.readLine();
			params = parse_params(input, delim);
			
			// Store the parameters in a hasmap
			for ( p = 0; p < params.length; p++ )
			{
				String[] param = params[p].split(":");
				parameters.put(param[0], Double.parseDouble(param[1]));
			}
			
			while ( input.compareTo("#endheader") != 0 )
			{
				input = in.readLine();
			}
		} 
		catch (IOException e) 
		{
			System.err.println("Error reading header data from: "+inputPath+inputFile+"; "+e);
		}
		
		// Read the parameter hashmap into local variables
		int boids = parameters.get("boids").intValue();
		int loops = parameters.get("loops").intValue();
		int width = parameters.get("width").intValue();
		int height = parameters.get("height").intValue();
		double boidWidth = width/5;
		
		// Unused parameters - commented out
		//int 	k = parameters.get("k").intValue();
		//double 	maxv = parameters.get("maxv");
		//double 	acc = parameters.get("acc");
	
		// Read the boid data into the BoidModel
		_model = new BoidModel(loops);
		
		try
		{
//			input = in.readLine();
//			while ( input.compareTo("#boiddata") != 0 )
//			{
//				input = in.readLine();
//			}
			
			// Read boids into the model until they have all been read
			for( p = 0; p < loops; p++ )
			{
				for( b = 0; b < boids; b++ )
				{
					input = in.readLine();
					
					try
					{
						newBoid = readBoid(input, boidWidth, b+1);
						if(newBoid != null)
							_model.addBoid(newBoid, p);
					}
					catch (Exception e)
					{
						
					}
				}
			}
			
//			while ( input.compareTo("#endboiddata") != 0 )
//			{
//				input = in.readLine();
//			
//			}
		}
		catch (IOException e)
		{
			System.err.println("Error reading boid data from: "+inputPath+inputFile+"; "+e);
		}
		
		/*try
		{
			input = in.readLine();
			while ( input.compareTo("#timedata") != 0 )
			{
				input = in.readLine();
			}
			
			// TODO: Read any timing data
			
			while ( input.compareTo("#endtimedata") != 0 )
			{
				input = in.readLine();
			
			}
		}
		catch (IOException e)
		{
			System.err.println("Error reading time data from: "+inputPath+inputFile+"; "+e);
		}*/
		
		
		StringBuilder outputScript = new StringBuilder();
		
		outputScript.append("reset\n");
		outputScript.append("set palette defined (0 'black', 1 'green', 2 'yellow', 3 'orange', 4 'red')\n");
		outputScript.append("set terminal gif animate delay " + delay + "\n");
		outputScript.append("set output '"+outputPath+outputFile+"'\n");
		outputScript.append("set view 0,0\n"); // use 0,0 for top view, 35,35 for tilted
		outputScript.append("set xrange [0:"+width+"]\n");
		outputScript.append("set yrange [0:"+height+"]\n");
		outputScript.append("plot 0\n");
		
		// Generate the boid image files and output script for gnuplot
		for (int iIteration = 0; iIteration < loops; ++iIteration)
	    {
	         OverlayBoids overlay = _model.update();

	         String data = overlay.exportToGnuplot();

	         outputScript.append(data);

	         // TODO: Give the gnuplot command to plot the current plane in the gif
	         outputScript.append("replot\n");
	    }
		
		outputScript.append("set output\n");

	    // save the script
	    System.out.println(outputScript);

	    String filespecScript = (outputPath + gnuscriptFile);

	    try (BufferedWriter out = new BufferedWriter(new FileWriter(filespecScript)))
	    {
	    	out.write(outputScript.toString());
	    }
	    catch (IOException exception)
	    {
	    	System.err.println("cannot save script " + filespecScript + "; " + exception);
	    }
	}

	private Boid readBoid(String input, double radius, int id) 
	{
		String[] params = input.split(" ");
		Boid newBoid = null;
		
		try
		{
			newBoid = new Boid(id,
							   Double.parseDouble(params[1]),
							   Double.parseDouble(params[2]),
							   Double.parseDouble(params[3]),
							   Double.parseDouble(params[4]),
							   radius);
		}
		catch (Exception e)
		{
			System.err.println("Error parsing boid: " + e);
		}
		
		return newBoid;
	}

	private String[] parse_params(String input, String delim) 
	{
		String[] params = input.split(delim);
		return params;
	}
}
