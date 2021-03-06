/*
 * Copyright (C) 2012  Brockmann Consult GmbH (info@brockmann-consult.de)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation. This program is distributed in the hope it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

package org.esa.s3.synergy.util.cdl;

import org.esa.s3.synergy.util.TemplateResolver;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.text.MessageFormat;
import java.util.Properties;

/**
 * Dummy test data generator.
 */
public class TestDataGenerator {

    private static final String GLOBAL_ATTRIBUTES = "// global attributes:\n" +
                                                    "\t\t:Conventions = \"CF-1.4\" ;\n" +
                                                    "\t\t:title = \"S3 L1c dummy test data\" ;\n" +
                                                    "\t\t:institution = \"Brockmann Consult GmbH\" ;\n" +
                                                    "\t\t:source = \"Sentinel-3 S3L2PP\" ;\n" +
                                                    "\t\t:history = \"\" ;\n" +
                                                    "\t\t:comment = \"\" ;\n" +
                                                    "\t\t:references = \"S3-RS-TAF-SY-01247\" ;\n" +
                                                    "\t\t:contact = \"info@brockmann-consult.de\" ;\n" +
                                                    "\t\t:netCDF_version = \"netCDF-4\" ;\n" +
                                                    "\t\t:Data_set_name = \"${CDL_File_Basename}.nc\" ;";

    private static final String SUB_BAND_A = "A";
    private static final String SUB_BAND_B = "B";
    private static final String SUB_BAND_TDI = "A+B";

    private static final String NCGEN_PATH_DEFAULT = "/usr/local/bin/ncgen";
    private static final String TARGET_DIR_DEFAULT = "./tmp";

    private static File targetDir;
    private static String ncgenPath;

    public static void main(String[] args) {
        try {
            final File propertiesFile = new File("testdata.properties");
            if (propertiesFile.exists()) {
                System.getProperties().load(new FileReader(propertiesFile));
            }
            targetDir = new File(System.getProperty("targetDir", TARGET_DIR_DEFAULT));
            ncgenPath = System.getProperty("ncgenPath", NCGEN_PATH_DEFAULT);

            generateDatasets("OLC_RADIANCE_O", 1, 21);
            generateDatasets("SLST_NAD_RADIANCE_S", 1, 6);
            generateDatasets("SLST_ALT_RADIANCE_S", 1, 6);
            generateDataset("GEOLOCATION_REF");
            generateDataset("TIME_STAMP_OLC");
            generateDataset("PIX_ANNOT_OLC");
            generateSlstrFlagsDatasets();
            generateDataset("SUBS_ANNOT_GEOM_OLC");
            generateDataset("SUBS_ANNOT_METEO_OLC");
            generateDataset("SUBS_ANNOT_SLST_NAD");
            generateDataset("SUBS_ANNOT_SLST_ALT");
            generateDataset("GEN_INFO_OLC");
            generateDatasets("GEN_INFO_SLST_NAD_S", 1, 6);
            generateDatasets("GEN_INFO_SLST_ALT_S", 1, 6);
            generateDatasets("MISREGIST_OLC_Oref_O", 1, 16);
            generateDatasets("MISREGIST_OLC_Oref_O", 18, 21);
            generateDatasets("MISREGIST_SLST_NAD_Oref_S", 1, 6);
            generateDatasets("MISREGIST_SLST_NAD_Oref_S", 1, 6);
            generateSlstrCollocationDatasets();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static void generateSlstrFlagsDatasets() throws Exception {
        final Properties properties = createProperties();
        properties.setProperty("Template_File_Basename", "PIX_ANNOT_SLST_NAD");
        properties.setProperty("CDL_File_Basename", "${Template_File_Basename}_${Sub_Band}");
        generateDataset(properties);
        properties.setProperty("Template_File_Basename", "PIX_ANNOT_SLST_ALT");
        generateDataset(properties);
    }

    private static void generateSlstrCollocationDatasets() throws Exception {
        final Properties properties = createProperties();
        properties.setProperty("Template_File_Basename", "COLLOC_SLST_ALT");
        properties.setProperty("CDL_File_Basename", "${Template_File_Basename}_${Sub_Band}");
        generateDataset(properties);
    }

    private static void generateDataset(String templateName) throws Exception {
        final Properties properties = createProperties();
        properties.setProperty("Template_File_Basename", templateName);
        generateDataset(properties);
    }

    private static void generateDatasets(String templateName, int minChannel, int maxChannel) throws Exception {
        final Properties properties = createProperties();
        properties.setProperty("Template_File_Basename", templateName);
        properties.setProperty("CDL_File_Basename", "${Template_File_Basename}${i}");
        for (int i = minChannel; i <= maxChannel; i++) {
            properties.setProperty("i", String.format("%d", i));
            properties.setProperty("ii", String.format("%02d", i));
            generateDataset(properties);
        }
    }

    private static void generateDataset(Properties properties) throws Exception {
        final TemplateResolver resolver = new TemplateResolver(properties);
        final File cdlFile = new File(targetDir, resolver.resolveProperty("CDL_File_Basename") + ".cdl");
        BufferedReader reader = null;
        BufferedWriter writer = null;
        try {
            final String templateName = resolver.resolveProperty("Template_File_Basename");
            final InputStream is = TestDataGenerator.class.getResourceAsStream(templateName + ".cdl");
            reader = new BufferedReader(new InputStreamReader(is, "US-ASCII"));
            writer = new BufferedWriter(new FileWriter(cdlFile));
            String line = reader.readLine();
            while (line != null) {
                line = resolver.resolve(line);
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
        generateNcFile(cdlFile);
    }

    private static Properties createProperties() {
        final Properties properties = new Properties();
        properties.setProperty("Global_Attributes", GLOBAL_ATTRIBUTES);
        properties.setProperty("Sub_Band", SUB_BAND_A);
        properties.setProperty("CDL_File_Basename", "${Template_File_Basename}");
        return properties;
    }

    private static void generateNcFile(File cdlFile) throws Exception {
        final File ncFile = new File(targetDir, cdlFile.getName().replace(".cdl", ".nc"));
        final String command = ncgenPath + " -k 3 -o " + ncFile.getPath() + " " + cdlFile.getPath();
        final Process process = Runtime.getRuntime().exec(command);
        if (process.waitFor() != 0) {
            throw new Exception(
                    MessageFormat.format("process <code>{0}</code> terminated with exit value {1}",
                                         command, process.exitValue()));
        }
    }
}
