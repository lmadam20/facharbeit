/*
Copyright (c) 2016-2017 Leon Maurice Adam.

This file is a part of rom-packer.

rom-packer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

rom-packer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with rom-packer.  If not, see <http://www.gnu.org/licenses/>.
*/

package facharbeit.z80.rompacker;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;

public class RomPacker {
	
	private String[] binaries;
	private String[] sources;
	
	public RomPacker(String[] files)
	{
		binaries = files;
		System.out.println("Number of binaries: " + files.length);
		sources = new String[binaries.length];
		parseToSources();
	}
	
	public void parseToSources()
	{
		for (int i = 0; i < binaries.length; i++)
		{
			String[] nameSplit = binaries[i].split("\\.");
			StringBuilder sourceBuilder = new StringBuilder();
			for (int j = 0; j < nameSplit.length - 1; j++)
				sourceBuilder.append(nameSplit[j]);
			sourceBuilder.append(".asm");
			
			sources[i] = sourceBuilder.toString();
			System.out.println("[" + i + "] " + binaries[i] + " -> " + sources[i]);
		}
		System.out.println();
	}
	
	public void process() throws Exception
	{
		File sourcePast, binaryPast;
		sourcePast = new File(sources[0]);
		binaryPast = new File(binaries[0]);
		File outFile = new File("packed.bin");
		FileOutputStream outStream = new FileOutputStream(outFile);
		
		System.out.println("Copying binary 0 to output file...");
		FileInputStream inStream0 = new FileInputStream(binaryPast);
		byte[] buf0 = new byte[(int) binaryPast.length()];
		inStream0.read(buf0);
		inStream0.close();
		outStream.write(buf0);
		outStream.flush();
		
		for (int i = 0; i < binaries.length - 1; i++)
		{
			File source2, binary2;
			source2 = new File(sources[i + 1]);
			binary2 = new File(binaries[i + 1]);
			
			BufferedReader source1Rdr, source2Rdr;
			source1Rdr = new BufferedReader(new FileReader(sourcePast));
			source2Rdr = new BufferedReader(new FileReader(source2));
			
			int offset1 = getOffset(source1Rdr);
			int length1 = (int) binaryPast.length();
			int offset2 = getOffset(source2Rdr);
			
			int paddingLength = offset2 - (offset1 + length1);
			System.out.println("Calculated padding between " + i + " and " + (i + 1) + ": " + paddingLength);
			
			source1Rdr.close();
			source2Rdr.close();
			
			System.out.println("Writing padding...");
			byte[] buf = new byte[paddingLength];
			for (int j = 0; j < paddingLength; j++) buf[j] = (byte) 0x76;
			outStream.write(buf);
			
			System.out.println("Copying binary " + (i + 1) + "...\n");
			buf = new byte[(int) binary2.length()];
			FileInputStream binary2Rdr = new FileInputStream(binary2);
			binary2Rdr.read(buf);
			binary2Rdr.close();
			
			outStream.write(buf);
			outStream.flush();
			
			sourcePast = source2;
			binaryPast = binary2;
		}
		outStream.close();
	}
	
	private int getOffset(BufferedReader reader) throws NumberFormatException, IOException
	{
		String line;
		while ((line = reader.readLine()) != null)
		{
			String[] splitted = line.split(" ");
			if (splitted[0].trim().equalsIgnoreCase("org"))
			{
				if (splitted[1].startsWith("$"))
				{
					splitted[1] = splitted[1].substring(1);
					return Integer.parseInt(splitted[1], 16);
				}
				return Integer.parseInt(splitted[1]);
			}
		}
		return -1;
	}
	
	public String[] getBinaries()
	{
		return binaries;
	}
	
	public String[] getSources()
	{
		return sources;
	}
}
