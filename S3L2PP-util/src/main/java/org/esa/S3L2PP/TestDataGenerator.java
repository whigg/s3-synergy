package org.esa.S3L2PP;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.text.MessageFormat;

/**
 * Test data generator.
 */
public class TestDataGenerator {

    private static final String NCGEN_PATH_DEFAULT = "/usr/local/bin/ncgen";

    public static void main(String[] args) {
        try {
            generateOlciRadianceDatasets();
            generateSlstrRadianceDatasets();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static void generateOlciRadianceDatasets() throws Exception {
        generateDatasets(1, 21, "OLC_RADIANCE_O");
     }

   private static void generateSlstrRadianceDatasets() throws Exception {
       generateDatasets(1, 6, "SLST_NAD_RADIANCE_S");
       generateDatasets(1, 6, "SLST_ALT_RADIANCE_S");
   }

    private static void generateDatasets(int first, int last, String template) throws Exception {
        for (int i = first; i <= last; i++) {
            final File cdlFile = new File("testdata/cdl", template + i + ".cdl");
            BufferedReader reader = null;
            BufferedWriter writer = null;
            try {
                final InputStream is = TestDataGenerator.class.getResourceAsStream(template + ".cdl");
                reader = new BufferedReader(new InputStreamReader(is, "US-ASCII"));
                writer = new BufferedWriter(new FileWriter(cdlFile));
                String line = reader.readLine();
                while (line != null) {
                    line = line.replaceAll("\\$\\{i\\}", String.valueOf(i));
                    if (i < 10) {
                        line = line.replaceAll("\\$\\{ii\\}", "0" + String.valueOf(i));
                    } else {
                        line = line.replaceAll("\\$\\{ii\\}", String.valueOf(i));
                    }
                    writer.write(line + "\n");
                    line = reader.readLine();
                }
            } finally {
                try {
                    if (writer != null) {
                        writer.close();
                    }
                    if (reader != null) {
                        reader.close();
                    }
                } catch (IOException e) {
                    // ignore
                }
            }
            final File ncFile = new File("testdata/nc", cdlFile.getName().replace(".cdl", ".nc"));
            final String ncgenPath = System.getProperty("ncgenPath", NCGEN_PATH_DEFAULT);
            final String command = ncgenPath + " -k 3 -o " + ncFile.getPath() + " " + cdlFile.getPath();
            final Process process = Runtime.getRuntime().exec(command);
            if (process.waitFor() != 0) {
                throw new Exception(
                        MessageFormat.format("process <code>{0}</code> terminated with exit value {1}",
                                             command, process.exitValue()));
            }
        }
    }
}

