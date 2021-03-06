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

package org.esa.s3.synergy.util.dictionary;

import org.esa.s3.synergy.util.TemplateResolver;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.List;
import java.util.Properties;

public class DictionaryGenerator {

    private static final String SY1 = "SY1";
    private static final String SY2 = "SY2";
    // todo - rq/ts-20110126: use 'VGP' for VGT P
    // todo - rq/ts-20110126: use 'VGS' for VGT S

    private static String targetBaseDir;
    private static Properties segmentNames;

    public static void main(String[] args) throws Exception {
        if (args.length == 0) {
            throw new IllegalArgumentException("Base output path needed as argument.");
        }
        targetBaseDir = args[0];

        loadSegmentNames();

        generateDatasets(SY1, "GEN_INFO_OLC_", 1, 18);
        generateDatasets(SY1, "GEN_INFO_SLST_ALT_S", 25, 30, 1);
        generateDatasets(SY1, "GEN_INFO_SLST_NAD_S", 19, 24, 1);
        generateDataset(SY1, "GEOLOCATION_REF");
        generateDatasets(SY1, "MISREGIST_OLC_Oref_O", 1, 21, 1, new int[]{14, 15, 20});
        generateDatasets(SY1, "OLC_RADIANCE_O", 1, 21, 1, new int[]{14, 15, 20});
        generateDataset(SY1, "PIX_ANNOT_OLC");
        generateDataset(SY1, "PIX_ANNOT_SLST_ALT_A");
        generateDataset(SY1, "PIX_ANNOT_SLST_NAD_A");
        generateSLSTRadianceDatasets("SLST_ALT_RADIANCE_S", 24);
        generateSLSTRadianceDatasets("SLST_NAD_RADIANCE_S", 18);
        generateDataset(SY1, "SUBS_ANNOT_GEOM_OLC");
        generateDataset(SY1, "SUBS_ANNOT_METEO_OLC");
        generateDataset(SY1, "SUBS_ANNOT_SLST_ALT");
        generateDataset(SY1, "SUBS_ANNOT_SLST_NAD");
        generateDataset(SY1, "TIME_STAMP_OLC");

        generateSDRDataset(1, 30);
        generateDataset(SY2, "L2_SYN_aerosol_angstrom_exponent", "a550");
        generateDataset(SY2, "L2_SYN_aerosol_model_index_number", "amin");
        generateDataset(SY2, "L2_SYN_aerosol_optical_thickness", "t550");
        generateDataset(SY2, "L2_SYN_geo-location", "geolocation");
        generateDataset(SY2, "L2_SYN_geophysical_atmospheric_data", "tiepoints_meteo");
        generateDataset(SY2, "L2_SYN_OLCI_sub-sampled_annotations", "tiepoints_olci");
        generateDataset(SY2, "L2_SYN_SLSTR_nadir_sub-sampled_annotations", "tiepoints_slstr_n");
        generateDataset(SY2, "L2_SYN_SLSTR_oblique_sub-sampled_annotations", "tiepoints_slstr_o");
        generateDataset(SY2, "L2_SYN_status_flags", "flags");
        generateDataset(SY2, "L2_SYN_time_stamps", "time");
        generateDataset(SY2, "dimensions");
    }

    private static void loadSegmentNames() throws IOException {
        segmentNames = new Properties();
        segmentNames.load(DictionaryGenerator.class.getResourceAsStream("segment_names.properties"));
    }

    private static void generateSLSTRadianceDatasets(String templateName, int offset) throws Exception {
        String targetDir = targetBaseDir + File.separator + SY1;
        final Properties properties = new Properties();
        setSegmentName(templateName, properties);
        properties.setProperty("target_dir", targetDir);
        properties.setProperty("Template_File_Basename", templateName);
        for (int i = 1; i <= 6; i++) {
            properties.setProperty("i", String.format("%d", i));
            properties.setProperty("j", String.format("%d", i + offset));
            properties.setProperty("dataset_name", templateName + i);
            generateDataset(properties);
        }
    }

    private static void generateDatasets(String level, String templateFileBasename, int minChannel, int maxChannel,
                                         int start, int[] excludes) throws Exception {
        final String targetDir = targetBaseDir + File.separator + level;
        final Properties properties = new Properties();
        setSegmentName(templateFileBasename, properties);
        properties.setProperty("target_dir", targetDir);
        properties.setProperty("Template_File_Basename", templateFileBasename);
        for (int i = minChannel, excludeCount = 0; i <= maxChannel; i++) {
            if (isExcluded(excludes, i)) {
                start++;
                excludeCount++;
                continue;
            }
            properties.setProperty("i", String.format("%d", i - excludeCount));
            properties.setProperty("dataset_name", templateFileBasename + start);
            generateDataset(properties);
            start++;
        }
    }

    private static boolean isExcluded(int[] excludes, int i) {
        for (int exclude : excludes) {
            if (i == exclude) {
                return true;
            }
        }
        return false;
    }

    private static void generateDatasets(String level, String templateName, int minChannel, int maxChannel,
                                         int start) throws Exception {
        generateDatasets(level, templateName, minChannel, maxChannel, start, new int[0]);
    }

    private static void generateDataset(String level, String templateName) throws Exception {
        generateDataset(level, templateName, templateName);
    }

    private static void generateDataset(String level, String templateName, String datasetName) throws Exception {
        final Properties properties = new Properties();
        String targetDir = targetBaseDir + File.separator + level;
        setSegmentName(templateName, properties);
        properties.setProperty("Template_File_Basename", templateName);
        properties.setProperty("target_dir", targetDir);
        properties.setProperty("dataset_name", datasetName);
        generateDataset(properties);
    }

    private static void setSegmentName(String templateName, Properties properties) {
        String segmentName = segmentNames.getProperty(templateName);
        if (segmentName != null) {
            properties.setProperty("segmentName", segmentName);
        }
    }

    private static void generateDatasets(String level, String templateName, int minChannel, int maxChannel) throws
                                                                                                            Exception {
        generateDatasets(level, templateName, minChannel, maxChannel, minChannel);
    }

    private static void generateSDRDataset(int minChannel, int maxChannel) throws Exception {
        InputStream csv = DictionaryGenerator.class.getResourceAsStream("dictionary/wavelengths.csv");
        final CsvReader csvReader = new CsvReader(new InputStreamReader(csv), new char[]{','});
        final List<String[]> wavelengths = csvReader.readStringRecords();
        final Properties properties = new Properties();
        String targetDir = targetBaseDir + File.separator + SY2;
        setSegmentName("L2_SYN_Surface_Directional_Reflectance", properties);
        properties.setProperty("target_dir", targetDir);
        properties.setProperty("Template_File_Basename", "L2_SYN_Surface_Directional_Reflectance");
        properties.setProperty("SDR", "SDR_${i}");
        properties.setProperty("SDR_er", "SDR_${i}_er");
        for (int i = minChannel; i <= maxChannel; i++) {
            String[] wavelengthSet = wavelengths.get(i - 1);
            float temp = Float.parseFloat(wavelengthSet[1]);
            int roundedCentralWavelength = Math.round(temp);
            properties.setProperty("central_wavelength", wavelengthSet[1]);
            properties.setProperty("min_wavelength", wavelengthSet[2]);
            properties.setProperty("max_wavelength", wavelengthSet[3]);
            String suffix = "";
            if (i > 18) {
                suffix = "n";
            }
            if (i > 24) {
                suffix = "o";
            }
            if (roundedCentralWavelength < 1000) {
                properties.setProperty("dataset_name", "r0" + roundedCentralWavelength + suffix);
            } else {
                properties.setProperty("dataset_name", "r" + roundedCentralWavelength + suffix);
            }
            properties.setProperty("i", String.format("%d", i));
            generateDataset(properties);
        }
    }

    private static void generateDataset(Properties properties) throws Exception {
        final TemplateResolver resolver = new TemplateResolver(properties);
        final String targetDir = properties.getProperty("target_dir");
        final File targetFile = new File(targetDir, resolver.resolveProperty("dataset_name") + ".xml");
        BufferedReader reader = null;
        BufferedWriter writer = null;
        try {
            final String templateFileBasename = resolver.resolveProperty("Template_File_Basename");
            final InputStream is = DictionaryGenerator.class.getResourceAsStream(
                    "dictionary/" + templateFileBasename + ".xml");
            reader = new BufferedReader(new InputStreamReader(is, "US-ASCII"));
            writer = new BufferedWriter(new FileWriter(targetFile));
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
    }
}
