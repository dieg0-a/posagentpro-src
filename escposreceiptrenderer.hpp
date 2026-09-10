#pragma once

#include "json/single_include/nlohmann/json.hpp"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>

#include <QString>
#include <QStringConverter>
#include <QByteArray>

#include <QImage>
#include <QByteArray>
#include <QBuffer>

class EscPosReceiptRenderer
{
public:
    using json = nlohmann::json;
    using ByteStream = std::vector<std::uint8_t>;

    struct Options
    {
        // Characters per line using the normal ESC/POS font.
        //
        // Typical:
        //   58mm -> 32
        //   80mm -> 48
        int charsPerLine = 48;

        // Width of raster images in printer dots.
        //
        // Typical:
        //   58mm -> 384
        //   80mm -> 576
        int imageWidth = 576;

        // Center the logo.
        bool centerLogo = true;

        // Cut paper after the receipt.
        bool cutPaper = true;

        // Feed before cutting.
        int feedBeforeCut = 3;

        // Enable bold for important values.
        bool useBold = true;

        std::string encoderString = "IBM437";
    };

    explicit EscPosReceiptRenderer(Options options)
        : m_options(options)
    {
    }

    ByteStream render(const json& receipt)
    {
        ByteStream out;

        initialize(out);

        renderHeader(out, receipt);
        renderCustomer(out, receipt);
        renderLines(out, receipt);
        renderTotals(out, receipt);
        renderPayments(out, receipt);
        renderFooter(out, receipt);
        renderAfterFooter(out, receipt);

        if (m_options.feedBeforeCut > 0)
            feed(out, m_options.feedBeforeCut);

        if (m_options.cutPaper)
            cut(out);

        return out;
    }

private:

    Options m_options;

    std::string encodeString(const std::string &s) {
        // Create an encoder for a specific code page (e.g., Windows-1252 or Shift-JIS)
        QStringEncoder encoder(m_options.encoderString);

        // Convert QString to encoded QByteArray
        QByteArray encoded_string = encoder(s.c_str());
        return encoded_string.constData();
    }

    // ================================================================
    // ESC/POS primitives
    // ================================================================

    static void append(
        ByteStream& out,
        std::initializer_list<std::uint8_t> bytes)
    {
        out.insert(out.end(), bytes.begin(), bytes.end());
    }

    static void append(
        ByteStream& out,
        const std::string& str)
    {
        out.insert(
            out.end(),
            str.begin(),
            str.end()
        );
    }

    void text(
        ByteStream& out,
        const std::string& str,
        bool encode=true)
    {
        if (encode){
            append(out, this->encodeString(str));
        }
        else {
            append(out, str);
        }
    }

    static void newline(ByteStream& out)
    {
        out.push_back('\n');
    }

    static void initialize(ByteStream& out)
    {
        // ESC @
        append(out, {0x1B, 0x40});
    }

    static void alignLeft(ByteStream& out)
    {
        // ESC a 0
        append(out, {0x1B, 0x61, 0x00});
    }

    static void alignCenter(ByteStream& out)
    {
        // ESC a 1
        append(out, {0x1B, 0x61, 0x01});
    }

    static void alignRight(ByteStream& out)
    {
        // ESC a 2
        append(out, {0x1B, 0x61, 0x02});
    }

    static void bold(
        ByteStream& out,
        bool enabled)
    {
        // ESC E n
        append(
            out,
            {
                0x1B,
                0x45,
                static_cast<std::uint8_t>(enabled ? 1 : 0)
            }
        );
    }

    static void fontSize(
        ByteStream& out,
        int width,
        int height)
    {
        /*
         * GS ! n
         *
         * Low nibble  = height multiplier - 1
         * High nibble = width multiplier - 1
         *
         * 1x1 => 0x00
         * 2x1 => 0x10
         * 1x2 => 0x01
         * 2x2 => 0x11
         */

        width = std::clamp(width, 1, 8);
        height = std::clamp(height, 1, 8);

        const auto value =
            static_cast<std::uint8_t>(
                ((width - 1) << 4) |
                (height - 1)
            );

        append(out, {0x1D, 0x21, value});
    }

    static void normalFont(ByteStream& out)
    {
        fontSize(out, 1, 1);
    }

    static void feed(
        ByteStream& out,
        int lines)
    {
        if (lines <= 0)
            return;

        append(
            out,
            {
                0x1B,
                0x64,
                static_cast<std::uint8_t>(
                    std::min(lines, 255)
                )
            }
        );
    }

    static void cut(ByteStream& out)
    {
        // GS V 0
        append(out, {0x1D, 0x56, 0x00});
    }

    void horizontalRule(
        ByteStream& out,
        int width)
    {
        text(
            out,
            std::string(width, '-')
        );
        newline(out);
    }


    // ================================================================
    // Text helpers
    // ================================================================

    static const std::string jsonString(
        const json& object,
        const char* key,
        const std::string fallback = "")
    {
        if (!object.contains(key) ||
            object[key].is_null())
        {
            return fallback;
        }

        return object[key].is_string() ? object[key].get_ref<const std::string&>() : fallback;
    }

    static double jsonNumber(
        const json& object,
        const char* key,
        double fallback = 0.0)
    {
        if (!object.contains(key) ||
            object[key].is_null())
        {
            return fallback;
        }

        return object[key].get<double>();
        return object[key].is_number() ? object[key].get<const double>() : fallback;
    }

    static bool jsonBool(
        const json& object,
        const char* key,
        bool fallback = false)
    {
        if (!object.contains(key) ||
            object[key].is_null())
        {
            return fallback;
        }

        return object[key].is_boolean() ? object[key].get<bool>() : fallback;
    }

    static std::string formatAmount(double amount)
    {
        std::ostringstream ss;

        ss << std::fixed
           << std::setprecision(2)
           << amount;

        return ss.str();
    }

    /*
     * Right-align a value while keeping a label on the left.
     *
     * Example:
     *
     * "TOTAL                         $3000.00"
     */
    std::string twoColumn(
        const std::string& left,
        const std::string& right) const
    {
        const int width = m_options.charsPerLine;

        if (
            static_cast<int>(left.length()) +
            static_cast<int>(right.length()) + 1
            >= width
        )
        {
            return left + " " + right;
        }

        const int spaces =
            width -
            static_cast<int>(left.length()) -
            static_cast<int>(right.length());

        return left +
               std::string(spaces, ' ') +
               right;
    }

    void printTwoColumn(
        ByteStream& out,
        const std::string& left,
        const std::string& right,
        bool encode_strings = true)
    {
        if (encode_strings) {
            auto encodedLeft = this->encodeString(left);
            auto encodedRight = this->encodeString(right);
            text(out, twoColumn(encodedLeft, encodedRight), false);
        }
        else {
            text(out, twoColumn(left, right), false);
        }
        newline(out);
    }


    // ================================================================
    // Header
    // ================================================================

    void renderHeader(
        ByteStream& out,
        const json& receipt)
    {
        const auto& header =
            receipt.value("header", json::object());

        const auto& company =
            receipt.value("company", json::object());

        const auto& config =
            receipt.at("order").value("config", json::object());

        const bool printLogo = config.value("receiptPrintLogo", false);

        // const auto &images = receipt.value("images", json::object());
        const auto logo = printLogo ? jsonString(receipt, "logoUrl", "") : "";
       

        /*
         * Logo
         *
         * The Odoo serializer supplies the data URL:
         *
         * data:image/png;base64,...
         *
         * We decode it and print it as a raster image.
         */
        if (!logo.empty())
        {
            if (m_options.centerLogo)
                alignCenter(out);

            printDataUrlImage(
                out,
                logo
            );

            feed(out, 1);
        }
        else {
            alignCenter(out);

            if (m_options.useBold)
                bold(out, true);

            fontSize(out, 1, 1);

            // const std::string companyName =
            //     company.at("name").is_string() ? company.value("name", "") : "";
            const std::string companyName = jsonString(company, "name");

            if (!companyName.empty())
            {
                text(out, companyName);
                newline(out);
            }

            if (m_options.useBold)
                bold(out, false);
        }

        const auto order =
            receipt.value("order", json::object());

        // const auto config =
        //     order.value("config", json::object());

        const bool isVat =
            header.value("isVat", false);

        text(
            out,
            isVat
                ? "VAT Ticket"
                : "Ticket"
        );


        //newline(out);

        // const std::string reference =
        //     header.at("posReference").is_string() ? header.value("posReference", "") : "";
        const std::string reference = jsonString(order, "posReference");

        if (!reference.empty())
        {
            text(out, " ");
            text(out, reference);
            newline(out);
        }
        else {
            newline(out);
        }

        const std::string date = jsonString(order, "formattedDateOrder");

        // const std::string date =
        //     header.value(
        //         "formattedDateOrder",
        //         ""
        //     );

        if (!date.empty())
        {
            text(out, date);
            newline(out);
        }

        /*
         * Receipt header configured in POS.
         */

        const std::string receiptHeader = jsonString(header, "header");
        // const auto receiptHeader = header.at("header").is_string() ? 
        //     header.value("header", "") : "";

        if (!receiptHeader.empty())
        {
            newline(out);

            printMultiline(
                out,
                receiptHeader
            );
        }

        const std::string cashier = jsonString(header, "cashier");

        // const std::string cashier =
        //     header.value("cashier", "");

        if (!cashier.empty())
        {
            newline(out);
            text(out, cashier);
            newline(out);
        }

        const std::string tracking = jsonString(header, "trackingNumber");

        // const std::string tracking =
        //     header.value(
        //         "trackingNumber",
        //         ""
        //     );

        if (!tracking.empty())
        {
            newline(out);

            if (m_options.useBold)
                bold(out, true);

            text(out, tracking);
            newline(out);

            if (m_options.useBold)
                bold(out, false);
        }

        newline(out);

        alignLeft(out);
    }


    // ================================================================
    // Customer
    // ================================================================

    void renderCustomer(
        ByteStream& out,
        const json& receipt)
    {
        const auto& header =
            receipt.value("header", json::object());

        const auto& customer =
            header.value(
                "customer",
                json::object()
            );

        if (customer.is_null() ||
            customer.empty())
        {
            return;
        }

        const auto name = jsonString(customer, "name");

        // const auto name =
        //     customer.value("name", "");

        const auto parent = jsonString(customer, "parentName");

        // const auto parent =
        //     customer.value("parentName", "");

        const auto address = jsonString(customer, "contactAddress");

        // const auto address =
        //     customer.value(
        //         "contactAddress",
        //         ""
        //     );

        const auto vat = jsonString(customer, "vat");

        // const auto vat =
        //     customer.value("vat", "");

        if (!parent.empty())
        {
            text(out, parent);
            text(out, ", ");
        }

        if (!name.empty())
        {
            text(out, name);
            newline(out);
        }

        if (!address.empty())
        {
            text(out, address);
            newline(out);
        }

        if (!vat.empty())
        {
            text(out, vat);
            newline(out);
        }

        newline(out);
    }


    // ================================================================
    // Order lines
    // ================================================================

    void renderLines(
        ByteStream& out,
        const json& receipt)
    {
        const auto lines =
            receipt.value(
                "display",
                json::object()
            ).value(
                "lines",
                json::array()
            );

        if (lines.empty())
            return;

        for (const auto& line : lines)
        {
            renderLine(out, line);
        }

        /*
         * General customer note.
         */
        const auto generalNote =
            receipt.value(
                "display",
                json::object()
            ).value(
                "generalCustomerNote",
                json(nullptr)
            );

        if (!generalNote.is_null() &&
            generalNote.is_array() &&
            !generalNote.empty())
        {
            newline(out);

            if (m_options.useBold)
                bold(out, true);

            text(out, "Note:");
            newline(out);

            if (m_options.useBold)
                bold(out, false);

            for (const auto& note : generalNote)
            {
                text(out, "  ");
                text(out, note.get<std::string>());
                newline(out);
            }
        }

        newline(out);
    }

    void renderLine(
        ByteStream& out,
        const json& line)
    {
        const auto vals =
            line.value(
                "screenValues",
                json::object()
            );

        if (vals.empty())
            return;

        const std::string quantity =
            vals.value("unitPart", "");

        const std::string decimal =
            vals.value("decimalPart", "");

        const std::string name =
            vals.value("name", "");

        const std::string price =
            vals.value("price", "");

        // const auto &taxGroup_element =
        //     vals.at("taxGroup");

        const std::string taxGroup = jsonString(vals, "taxGroup");

        /*
         * First line:
         *
         * 2.5 x Product Name             $12.50
         */
        std::string qty =
            quantity + decimal;

        if (!qty.empty())
            qty += " x ";

        std::string productName =
            qty + name;

        if (price.length() >= m_options.charsPerLine - 5) {
            text(out, "Price too long cannot print");
            newline(out);
            return;
        }

        std::string priceEncoded = encodeString(price);
        std::string productNameEncoded = encodeString(productName);
        std::string firstLine =  productNameEncoded.substr(0,
                                                  std::min((unsigned long)(m_options.charsPerLine-priceEncoded.length() - 1),
                                           productNameEncoded.length()));
        std::string lastLine = productNameEncoded.substr(std::min((unsigned long)(m_options.charsPerLine-priceEncoded.length() - 1),
                                                                  productNameEncoded.length()));

        if (!priceEncoded.empty())
        {
            printTwoColumn(
                out,
                firstLine,
                priceEncoded,
                false
                );
        }
        else
        {
            text(out, firstLine);
            newline(out);
        }




        while (lastLine.length() > 0) {
            if (lastLine.length() > m_options.charsPerLine-1) {
                text(out,
                     lastLine.substr(0,
                                     (unsigned long)(m_options.charsPerLine-1)), false);
                lastLine = lastLine.substr((m_options.charsPerLine-1));
            }
            else {
                text(out, lastLine, false);
                lastLine = "";
                newline(out);
                break;
            }
            newline(out);
        }

        // if (!price.empty())
        // {
        //     printTwoColumn(
        //         out,
        //         lastLine,
        //         price
        //     );
        // }
        // else
        // {
        //     text(out, lastLine);
        //     newline(out);
        // }

        /*
         * Attribute string
         */
        // const auto &attribute_element =
        //     vals.at(
        //         "attributeString"
        //     );
        // const std::string attribute = attribute_element.is_string() ? attribute_element.get<std::string>() : "";

        const std::string attribute  = jsonString(vals, "attributeString");

        if (!attribute.empty())
        {
            text(out, "    ");
            text(out, attribute);
            newline(out);
        }

        /*
         * Tax group
         */
        if (!taxGroup.empty())
        {
            text(out, "    ");
            text(out, taxGroup);
            newline(out);
        }

        /*
         * Price per unit
         */
        const std::string priceUnit =
            vals.value(
                "displayPriceUnit",
                ""
            );

        if (!priceUnit.empty())
        {
            text(out, "    ");
            text(out, priceUnit);
            newline(out);
        }

        /*
         * Discount
         */
        const std::string discount =
            vals.value(
                "discount",
                ""
            );

        if (!discount.empty())
        {
            text(out, "    ");
            text(out, "Discount ");
            text(out, discount);
            text(out, "%");
            newline(out);

            const std::string noDiscount =
                vals.value(
                    "noDiscountPrice",
                    ""
                );

            if (!noDiscount.empty())
            {
                text(out, "    ");
                text(out, "Regular: ");
                text(out, noDiscount);
                newline(out);
            }
        }

        /*
         * Customer note
         */
        // const auto &customerNote_element = line.at("customer_note");
        // const std::string customerNote = customerNote_element.is_string() ? customerNote_element.get<std::string>() : "";

        const std::string customerNote = jsonString(line, "customer_note");
        if (!customerNote.empty())
        {
            text(out, "    Note: ");
            text(out, customerNote);
            newline(out);
        }

        /*
         * Lot / serial numbers
         */
        const auto lots =
            vals.value(
                "lotLines",
                json(nullptr)
            );

        if (
            !lots.is_null() &&
            lots.is_array() &&
            !lots.empty()
        )
        {
            text(out, "    Lot/Serial:");
            newline(out);

            for (const auto& lot : lots)
            {
                if (lot.is_string())
                {
                    text(out, "      ");
                    text(out, lot.get<std::string>());
                    newline(out);
                }
                else if (lot.is_object())
                {
                    /*
                     * Keep this tolerant of the exact Odoo lot object
                     * representation.
                     */
                    if (lot.contains("lot_name"))
                    {
                        text(out, "      ");
                        text(
                            out,
                            lot["lot_name"]
                                .get<std::string>()
                        );
                        newline(out);
                    }
                }
            }
        }
    }


    // ================================================================
    // Totals
    // ================================================================

    void renderTotals(
        ByteStream& out,
        const json& receipt)
    {
        const auto order =
            receipt.value(
                "order",
                json::object()
            );

        const auto prices =
            order.value(
                "prices",
                json::object()
            );

        const auto taxDetails =
            prices.value(
                "taxDetails",
                json(nullptr)
            );

        const auto &config = receipt.at("order").at("config");
        const bool receiptShowTaxDetails = config.value("receiptShowTaxDetails", false);

        /*
         * Subtotal
         */
        if (
            prices.contains(
                "currencyDisplayPriceExcl"
            ) && receiptShowTaxDetails
        )
        {
            horizontalRule(
                out,
                m_options.charsPerLine
                );
            const auto subtotal =
                prices[
                    "currencyDisplayPriceExcl"
                ].get<std::string>();

            printTwoColumn(
                out,
                "Subtotal",
                subtotal
            );
        }

        /*
         * Tax groups
         *
         * Mirrors the information in:
         *
         * order.prices.taxDetails
         */
        if (
            !taxDetails.is_null() &&
            taxDetails.value(
                "hasTaxGroups",
                false
            ) && receiptShowTaxDetails
        )
        {
            const auto subtotals =
                taxDetails.value(
                    "subtotals",
                    json::array()
                );

            for (const auto& subtotal : subtotals)
            {
                const auto groups =
                    subtotal.value(
                        "taxGroups",
                        json::array()
                    );

                for (const auto& group : groups)
                {
                    std::string label =
                        group.value(
                            "groupName",
                            ""
                        );

                    // const auto &groupLabel_element =
                    //     group.at(
                    //         "groupLabel"
                    //     );
                    // const std::string groupLabel = groupLabel_element.is_string() ? groupLabel_element.get<std::string>() : "";

                    const std::string groupLabel = jsonString(group, "groupLabel");

                    if (!groupLabel.empty())
                    {
                        label +=
                            " (" +
                            groupLabel +
                            ")";
                    }

                    /*
                     * The original receipt can optionally display:
                     *
                     * IVA (21%) on $1000
                     */
                    if (
                        !taxDetails.value(
                            "sameTaxBase",
                            true
                        )
                    )
                    {
                        const std::string base =
                            formatAmount(
                                group.value(
                                    "baseAmountCurrency",
                                    0.0
                                )
                            );

                        label +=
                            " on " + base;
                    }

                    const std::string amount =
                        formatAmount(
                            group.value(
                                "taxAmountCurrency",
                                0.0
                            )
                        );

                    const std::string amountFormatted = group.value(
                            "taxAmountCurrencyFormatted",
                            ""
                        );

                    printTwoColumn(
                        out,
                        label,
                        amountFormatted
                    );
                }
            }
        }

        horizontalRule(
            out,
            m_options.charsPerLine
        );

        /*
         * Total
         */
        const std::string total =
            prices.value(
                "currencyDisplayPriceIncl",
                ""
            );

        if (!total.empty())
        {
            if (m_options.useBold)
                bold(out, true);

            printTwoColumn(
                out,
                "TOTAL",
                total
            );

            if (m_options.useBold)
                bold(out, false);
        }

        /*
         * Rounding
         */
        if (
            !order["appliedRounding"].is_null() &&
            order.contains("appliedRounding")
        )
        {
            const double rounding =
                order.value(
                    "appliedRounding",
                    0.0
                );

            if (rounding != 0.0) {
                const auto &translated = receipt.value("translated", json::object());
                std::string rounding_label = jsonString(translated, "rounding");
                std::string topay_label = jsonString(translated, "topay");

                printTwoColumn(
                    out,
                    rounding_label,
                    formatAmount(rounding)
                    );

                printTwoColumn(
                    out,
                    topay_label,
                    order.value(
                        "roundedPriceInclFormatted",
                        ""
                        )
                    );
            }
        }

        /*
         * Discounts
         */
        const auto discount =
            order.value(
                "totalDiscount",
                json(nullptr)
            );

        if (
            !discount.is_null() &&
            discount.is_number()
        )
        {
            const double value =
                discount.get<double>();

            if (value != 0.0)
            {
                printTwoColumn(
                    out,
                    "Discounts",
                    formatAmount(value)
                );
            }
        }

        newline(out);
    }


    // ================================================================
    // Payments
    // ================================================================

    void renderPayments(
        ByteStream& out,
        const json& receipt)
    {
        const auto payments =
            receipt.value(
                "paymentLines",
                json::array()
            );

        for (const auto& payment : payments)
        {
            const auto method =
                payment.value(
                    "paymentMethod",
                    json::object()
                );

            const std::string name =
                method.value(
                    "name",
                    "Payment"
                );

            /*
             * The serializer stores the numeric amount here.
             *
             * Note that this may differ from the localized formatted
             * currency representation in the QWeb receipt.
             */
            const double amount =
                payment.value(
                    "amount",
                    0.0
                );

            const std::string amountFormatted = payment.value("amountFormatted", "");

            printTwoColumn(
                out,
                name,
                amountFormatted
            );
        }

        const auto order =
            receipt.value(
                "order",
                json::object()
            );

        // const double change = order.value("change", 0.0);
        if (
            order.value(
                "showChange",
                false
            )
        )
        {
            printTwoColumn(
                out,
                receipt.at("translated").value("change", ""),
                order.value("changeFormatted", "")
            );
        }

        newline(out);
    }


    // ================================================================
    // Footer
    // ================================================================

    void renderFooter(
        ByteStream& out,
        const json& receipt)
    {
        const auto footer =
            receipt.value(
                "footer",
                json::object()
            );

        // const auto &receiptFooter_element = footer.at("receiptFooter");
        // const std::string receiptFooter = receiptFooter_element.is_string() ? receiptFooter_element.get<std::string>() : "";
        
        const std::string receiptFooter = jsonString(footer, "receiptFooter");

        if (!receiptFooter.empty())
        {
            alignCenter(out);

            printMultiline(
                out,
                receiptFooter
            );

            newline(out);
        }

        const auto order =
            receipt.value(
                "order",
                json::object()
            );

        // const auto &shippingData_element = footer.at("formattedShippingDate");
        // const std::string shippingDate = shippingData_element.is_string() ? shippingData_element.get<std::string>() : "";
        const std::string shippingDate = jsonString(footer, "formattedShippingDate");

        if (!shippingDate.empty())
        {
            text(
                out,
                "Expected delivery: "
            );

            text(out, shippingDate);
            newline(out);

            newline(out);
        }

        /*
         * QR code
         */
        const auto portal =
            receipt.value(
                "images",
                json::object()
            );


        // const auto &qrCode_element = portal.at("qrCode");
        // const std::string qrCode = qrCode_element.is_string() ? qrCode_element.get<std::string>() : "";
        
        const std::string qrCode = jsonString(portal, "qrCode");

        if (!qrCode.empty())
        {
            alignCenter(out);

            printDataUrlImage(
                out,
                qrCode
            );

            newline(out);

            const auto orderObject =
                receipt.value(
                    "order",
                    json::object()
                );

            const std::string ticketCode =
                orderObject.value(
                    "ticketCode",
                    ""
                );

            if (!ticketCode.empty())
            {
                text(out, "Code: ");
                text(out, ticketCode);
                newline(out);
            }

            newline(out);
        }

        alignLeft(out);

        /*
         * Restore normal printer state.
         */
        bold(out, false);
        normalFont(out);
    }


    void renderAfterFooter(ByteStream& out,
                           const json& receipt) {
        const auto& company =
            receipt.value("company", json::object());

        const auto& order =
            receipt.value("order", json::object());

        const auto& config =
            order.value("config", json::object());
        std::string configName = jsonString(config, "name");
        // const std::string companyName =
        //     company.at("name").is_string() ? company.value("name", "") : "";
        const std::string companyName = jsonString(company, "name");
        const std::string companyEmail = jsonString(company, "email");
        const std::string companyWebsite = jsonString(company, "website");
        const std::string companyPhone = jsonString(company, "phone");
        const std::string companyStreet = jsonString(company, "street");
        const std::string companyCity = jsonString(company, "city");
        const std::string companyZIP = jsonString(company, "zip");
        const std::string vatText = jsonString(receipt, "vatText");

        const auto state = company.at("state");
        const std::string code =jsonString(state, "code");

        std::queue<std::string> left_side_stack = std::queue<std::string>();
        std::queue<std::string> right_side_stack = std::queue<std::string>();

        left_side_stack.push(configName);
        if (!companyStreet.empty()) left_side_stack.push(companyStreet);
        if (!companyCity.empty()) left_side_stack.push(companyCity);
        if (!code.empty()) left_side_stack.push(code);
        if (!companyZIP.empty()) left_side_stack.push(companyZIP);

        if (!vatText.empty()) right_side_stack.push(vatText);
        if (!companyPhone.empty()) right_side_stack.push(companyPhone);
        if (!companyEmail.empty()) right_side_stack.push(companyEmail);
        if (!companyWebsite.empty()) right_side_stack.push(companyWebsite);


        while (!left_side_stack.empty() || !right_side_stack.empty()) {
            printTwoColumn(out,
                           left_side_stack.empty() ? "" : left_side_stack.front(),
                           right_side_stack.empty() ? "" : right_side_stack.front()
                           );
            if (!left_side_stack.empty()) left_side_stack.pop();
            if (!right_side_stack.empty()) right_side_stack.pop();
        }
        newline(out);

        alignCenter(out);

        text(out, "Powered by Odoo");
        newline(out);

        alignLeft(out);

        /*
         * Restore normal printer state.
         */
        bold(out, false);
        normalFont(out);


    }


    // ================================================================
    // Multiline text
    // ================================================================

    void printMultiline(
        ByteStream& out,
        const std::string& textValue)
    {
        std::size_t start = 0;

        while (start <= textValue.size())
        {
            const auto end =
                textValue.find(
                    '\n',
                    start
                );

            if (end == std::string::npos)
            {
                text(
                    out,
                    textValue.substr(start)
                );

                newline(out);
                break;
            }

            text(
                out,
                textValue.substr(
                    start,
                    end - start
                )
            );

            newline(out);

            start = end + 1;
        }
    }


    // ================================================================
    // Image support
    // ================================================================

    static std::vector<std::uint8_t> base64Decode(
        const std::string& input)
    {
        static constexpr char table[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        std::vector<std::uint8_t> output;

        int val = 0;
        int valBits = -8;

        for (const unsigned char c : input)
        {
            if (c == '=')
                break;

            const char* pos =
                std::find(
                    std::begin(table),
                    std::end(table) - 1,
                    c
                );

            if (pos == std::end(table) - 1)
                continue;

            val =
                (val << 6) +
                static_cast<int>(
                    pos - table
                );

            valBits += 6;

            if (valBits >= 0)
            {
                output.push_back(
                    static_cast<std::uint8_t>(
                        (val >> valBits) & 0xFF
                    )
                );

                valBits -= 8;
            }
        }

        return output;
    }


    static std::vector<std::uint8_t>
    extractDataUrlBase64(
        const std::string& dataUrl)
    {
        const auto comma =
            dataUrl.find(',');

        if (comma == std::string::npos)
            throw std::runtime_error(
                "Invalid image data URL"
            );

        return base64Decode(
            dataUrl.substr(comma + 1)
        );
    }


    /*
     * Decode the image and print it.
     *
     * This function currently expects the decoded bytes to be a
     * 1-bit-per-pixel raster image.
     *
     * In the next layer, we should use a real image decoder such as
     * stb_image or OpenCV to convert PNG/JPEG/WebP -> monochrome bitmap.
     */
    void printDataUrlImage(
        ByteStream& out,
        const std::string& dataUrl)
    {
        if (dataUrl.empty())
            return;

        const auto imageData =
            extractDataUrlBase64(dataUrl);

        /*
         * IMPORTANT:
         *
         * imageData is currently PNG/JPEG/etc. compressed image data.
         *
         * ESC/POS cannot accept PNG/JPEG bytes directly with GS v 0.
         *
         * We therefore need:
         *
         *   PNG/JPEG
         *       ↓
         *   decoded bitmap
         *       ↓
         *   monochrome bitmap
         *       ↓
         *   ESC/POS raster bytes
         *
         * This is intentionally separated into decodeImage().
         */
        const Bitmap bitmap =
            decodeImage(
                imageData,
                m_options.imageWidth
            );

        printRasterBitmap(
            out,
            bitmap
        );
    }


    // ================================================================
    // Bitmap representation
    // ================================================================

    struct Bitmap
    {
        int width = 0;
        int height = 0;

        /*
         * One bit per pixel.
         *
         * Pixel (x,y):
         *
         *   data[y * stride + x / 8]
         *
         * Bit 7 is the left-most pixel.
         */
        int stride = 0;

        std::vector<std::uint8_t> data;
    };


    /*
     * Image decoder placeholder.
     *
     * Replace this with stb_image, libpng, etc.
     */
    // static Bitmap decodeImage(
    //     const std::vector<std::uint8_t>& imageData,
    //     int targetWidth)
    // {
    //     /*
    //      * This cannot be implemented correctly using the C++ standard
    //      * library alone because PNG/JPEG/WebP are compressed formats.
    //      *
    //      * See the implementation below for the recommended stb_image
    //      * integration.
    //      */

    //     (void)imageData;
    //     (void)targetWidth;

    //     throw std::runtime_error(
    //         "decodeImage() requires an image decoder"
    //     );
    // }

    static Bitmap decodeImage(
        const std::vector<std::uint8_t>& imageData,
        int targetWidth)
    {
        QImage image;

        if (!image.loadFromData(
                QByteArray(
                    reinterpret_cast<const char*>(
                        imageData.data()
                        ),
                    static_cast<int>(
                        imageData.size()
                        )
                    )))
        {
            throw std::runtime_error(
                "Unable to decode receipt image"
                );
        }

        /*
     * Scale while preserving aspect ratio.
     */
        if (image.width() > targetWidth)
        {
            image = image.scaledToWidth(
                targetWidth,
                Qt::SmoothTransformation
                );
        }

        image = image.convertToFormat(
            QImage::Format_Grayscale8
            );

        const int width = image.width();
        const int height = image.height();
        const int stride = (width + 7) / 8;

        Bitmap bitmap;

        bitmap.width = width;
        bitmap.height = height;
        bitmap.stride = stride;

        bitmap.data.resize(
            stride * height,
            0
            );

        /*
     * Simple threshold conversion.
     *
     * For thermal printers:
     *
     *   dark pixel = 1
     *   light pixel = 0
     */
        constexpr int threshold = 160;

        for (int y = 0; y < height; ++y)
        {
            const std::uint8_t* src =
                image.constScanLine(y);

            std::uint8_t* dst =
                bitmap.data.data() +
                y * stride;

            for (int x = 0; x < width; ++x)
            {
                const std::uint8_t gray =
                    src[x];

                if (gray < threshold)
                {
                    dst[x / 8] |=
                        static_cast<std::uint8_t>(
                            0x80 >> (x % 8)
                            );
                }
            }
        }

        return bitmap;
    }


    // ================================================================
    // ESC/POS raster image
    // ================================================================

    static void printRasterBitmap(
        ByteStream& out,
        const Bitmap& bitmap)
    {
        if (
            bitmap.width <= 0 ||
            bitmap.height <= 0 ||
            bitmap.data.empty()
        )
        {
            return;
        }

        const int widthBytes =
            (bitmap.width + 7) / 8;

        /*
         * GS v 0
         *
         * 1D 76 30 m xL xH yL yH
         */
        const std::uint8_t xL =
            widthBytes & 0xFF;

        const std::uint8_t xH =
            (widthBytes >> 8) & 0xFF;

        const std::uint8_t yL =
            bitmap.height & 0xFF;

        const std::uint8_t yH =
            (bitmap.height >> 8) & 0xFF;

        append(
            out,
            {
                0x1D,
                0x76,
                0x30,
                0x00,
                xL,
                xH,
                yL,
                yH
            }
        );

        for (int y = 0; y < bitmap.height; ++y)
        {
            const auto* row =
                bitmap.data.data() +
                y * bitmap.stride;

            out.insert(
                out.end(),
                row,
                row + widthBytes
            );
        }
    }
};
