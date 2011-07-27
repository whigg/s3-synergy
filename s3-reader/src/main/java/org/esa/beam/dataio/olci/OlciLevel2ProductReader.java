package org.esa.beam.dataio.olci;

import com.bc.ceres.core.ProgressMonitor;
import com.bc.ceres.glevel.MultiLevelImage;
import org.esa.beam.framework.dataio.AbstractProductReader;
import org.esa.beam.framework.dataio.ProductIO;
import org.esa.beam.framework.datamodel.Band;
import org.esa.beam.framework.datamodel.MetadataElement;
import org.esa.beam.framework.datamodel.Product;
import org.esa.beam.framework.datamodel.ProductData;
import org.esa.beam.framework.datamodel.TiePointGrid;
import org.esa.beam.util.ProductUtils;
import org.esa.beam.util.io.FileUtils;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Product reader responsible for reading OLCI L2 data products in SAFE format.
 *
 * @author Marco Peters
 * @since 1.0
 */
public class OlciLevel2ProductReader extends AbstractProductReader {

    private final Logger logger;
    private List<Product> measurementProducts;
    private Product geoCoordinatesProduct;
    private Product tiePointsProduct;

    public OlciLevel2ProductReader(OlciLevel2ProductReaderPlugIn readerPlugIn) {
        super(readerPlugIn);
        logger = Logger.getLogger(getClass().getSimpleName());

    }

    @Override
    protected Product readProductNodesImpl() throws IOException {
        File inputFile = getInputFile();
        OlciL2Manifest manifest = createManifestFile(inputFile);
        return createProduct(manifest);
    }


    @Override
    protected void readBandRasterDataImpl(int sourceOffsetX, int sourceOffsetY, int sourceWidth, int sourceHeight,
                                          int sourceStepX, int sourceStepY, Band destBand, int destOffsetX,
                                          int destOffsetY, int destWidth, int destHeight, ProductData destBuffer,
                                          ProgressMonitor pm) throws IOException {
        throw new IllegalStateException(String.format("No source to read from for band '%s'.", destBand.getName()));
    }

    private Product createProduct(OlciL2Manifest manifest) {
        measurementProducts = loadMeasurementProducts(manifest.getMeasurementFileNames());
        int width = measurementProducts.get(0).getSceneRasterWidth();
        int height = measurementProducts.get(0).getSceneRasterHeight();
        Product product = new Product(getProductName(), getProductType(), width, height, this);
        product.setStartTime(manifest.getStartTime());
        product.setEndTime(manifest.getStopTime());
        product.setFileLocation(getInputFile());
        attachMeasurementBands(measurementProducts, product);
        attachAnnotationData(manifest, product);
        return product;
    }

    private void attachAnnotationData(OlciL2Manifest manifest, Product product) {
        attachGeoCoodinatesToProduct(manifest.getGeoCoordinatesFileName(), product);
        attachTiePointsToProduct(manifest.getTiePointsFileName(), product);
    }

    private void attachTiePointsToProduct(String tiePointsFileName, Product product) {
        try {
            tiePointsProduct = readProduct(tiePointsFileName);
            Band[] tiePointBands = tiePointsProduct.getBands();
            MetadataElement metadataRoot = tiePointsProduct.getMetadataRoot();
            MetadataElement globalAttributes = metadataRoot.getElement("Global_Attributes");
            int subsampling = globalAttributes.getAttributeInt("subsampling_factor");
            for (Band band : tiePointBands) {
                MultiLevelImage sourceImage = band.getGeophysicalImage();

                int width = sourceImage.getWidth();
                int height = sourceImage.getHeight();
                float[] tiePointData = new float[width * height];
                sourceImage.getData().getSamples(0, 0, width, height, 0, tiePointData);
                TiePointGrid tiePointGrid = new TiePointGrid(band.getName(), band.getRasterWidth(),
                                                             band.getRasterHeight(), 0, 0, subsampling,
                                                             subsampling, tiePointData, true);
                product.addTiePointGrid(tiePointGrid);
            }

        } catch (IOException e) {
            String msg = String.format("Not able to read file '%s.", tiePointsFileName);
            logger.log(Level.WARNING, msg, e);
        }
    }

    private void attachGeoCoodinatesToProduct(String geoCoordinatesFileName, Product product) {
        try {
            geoCoordinatesProduct = readProduct(geoCoordinatesFileName);
            geoCoordinatesProduct.transferGeoCodingTo(product, null);
        } catch (IOException e) {
            String msg = String.format("Not able to read file '%s.", geoCoordinatesFileName);
            logger.log(Level.WARNING, msg, e);
        }
    }

    private void attachMeasurementBands(List<Product> measurementProducts, Product product) {
        for (Product bandProduct : measurementProducts) {
            Band[] bands = bandProduct.getBands();
            for (Band band : bands) {
                Band targetBand = ProductUtils.copyBand(band.getName(), bandProduct, product);
                targetBand.setSourceImage(band.getSourceImage());
            }
        }
    }

    private String getProductName() {
        return FileUtils.getFilenameWithoutExtension(getParentInputDirectory());
    }

    private String getProductType() {
        int typeStartindex = 3;
        int typeLength = "OL_2_WRR".length();
        return getProductName().substring(typeStartindex, typeStartindex + typeLength);
    }

    private List<Product> loadMeasurementProducts(List<String> measurementFileNames) {
        List<Product> products = new ArrayList<Product>();
        for (String fileName : measurementFileNames) {
            try {
                products.add(readProduct(fileName));
            } catch (IOException e) {
                String msg = String.format("Not able to read file '%s.", fileName);
                logger.log(Level.WARNING, msg, e);
            }
        }
        return products;
    }

    private Product readProduct(String fileName) throws IOException {
        File dataSetFile = new File(getParentInputDirectory(), fileName);
        Product product = ProductIO.readProduct(dataSetFile);
        if (product == null) {
            String msg = String.format("Could not read file '%s. No appropriate reader found.", fileName);
            throw new IOException(msg);
        }
        return product;
    }

    private OlciL2Manifest createManifestFile(File inputFile) throws IOException {
        InputStream manifestInputStream = new FileInputStream(inputFile);
        try {
            return new OlciL2Manifest(createXmlDocument(manifestInputStream));
        } finally {
            manifestInputStream.close();
        }

    }

    private Document createXmlDocument(InputStream inputStream) throws IOException {
        Document doc = null;
        try {
            doc = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse(inputStream);
        } catch (SAXException e) {
            e.printStackTrace();
        } catch (ParserConfigurationException e) {
            e.printStackTrace();
        }
        return doc;
    }

    private File getParentInputDirectory() {
        return getInputFile().getParentFile();
    }

    private File getInputFile() {
        return new File(getInput().toString());
    }

    @Override
    public void close() throws IOException {
        for (Product product : measurementProducts) {
            product.dispose();
        }
        if (geoCoordinatesProduct != null) {
            geoCoordinatesProduct.dispose();
        }
        if (tiePointsProduct != null) {
            tiePointsProduct.dispose();
        }
        super.close();

    }
}