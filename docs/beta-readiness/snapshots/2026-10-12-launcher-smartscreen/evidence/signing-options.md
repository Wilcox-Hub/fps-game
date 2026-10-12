# IRON SUN launcher: SmartScreen and publisher signing

Research date: 2026-10-12. Primary Microsoft sources only. No account, paid resource, certificate, purchase, terms acceptance, or Windows protection changes were performed.

## Finding

The reported warning is consistent with an unsigned, unfamiliar installer. The parent agent verified that the user's `IRON-SUN-Setup (2).exe` matches the published installer SHA256 `62C789A0ABD26F2CA127635AD61FDE7B41537AAD31C4C29A2D54FF754464C9F8`; Windows reports `NotSigned`. These session facts establish artifact identity and absent Authenticode signing. They do not establish safety, reproduce SmartScreen's online verdict, or imply publisher trust.

The prior Microsoft analyst's removal of an antivirus detection does not grant a publisher signature or clear SmartScreen reputation. Microsoft's developer FAQ expressly treats SmartScreen separately from Defender Antivirus/antimalware. The submission header remains In progress / Pending according to the parent agent's latest evidence; do not describe it as completed certification. [Microsoft software developer FAQ](https://learn.microsoft.com/en-us/defender-xdr/developer-faq)

Our release feed's RSA signature authenticates downloads to our launcher. Windows Authenticode is a separate publisher signature on executable files; a private release manifest key is not a publicly trusted code-signing certificate. [Microsoft explanation of code signing](https://azure.microsoft.com/en-us/products/artifact-signing)

## What signing changes

SmartScreen considers publisher/certificate reputation and the exact file hash. A valid signature can show the validated publisher and allow reputation to accumulate across releases. A new signed executable can still warn; unsigned revisions start their own file reputation. Microsoft's current guidance says EV certificates no longer provide an automatic SmartScreen bypass. It supplies no dependable threshold or promised clearance date. A self-signed certificate does not provide public SmartScreen trust. [Microsoft SmartScreen reputation guidance](https://learn.microsoft.com/en-us/windows/apps/package-and-deploy/smartscreen-reputation)

Therefore, do not buy EV solely to eliminate the warning, rename the EXE to imply trust, bundle a root certificate for friends to install, or promise that paying for signing immediately removes every prompt. The intended acceptance test is a fresh public download opened on an ordinary protected Windows PC, recording both the signature and actual first-run experience; signature validity alone is insufficient.

## Recommended direct-download route: Azure Artifact Signing

Artifact Signing is the current name of Microsoft's Trusted Signing service. Its official product FAQ lists Basic at **US$9.99/month**, including **5,000 signatures/month**, with **$0.005 per additional signature**; Premium is $99.99/month for 100,000. At unchanged Basic pricing, twelve paid months are $119.88 before taxes, extra signatures, or other Azure services. [Azure product FAQ and pricing](https://azure.microsoft.com/en-us/products/artifact-signing)

The pricing page's text extraction currently renders dollar amounts as placeholders, but confirms the Basic quota and says billing begins when the account is created. The stated $9.99 amount was independently present in the official product FAQ and Microsoft Windows signing guide. Confirm the selected region/currency and displayed Azure purchase price before provisioning. [Azure pricing and billing timing](https://azure.microsoft.com/en-us/pricing/details/artifact-signing/), [Microsoft signing comparison](https://learn.microsoft.com/en-us/windows/apps/package-and-deploy/code-signing-options)

Public Trust individual validation is available to developers located in the **United States or Canada**. The Azure billing account must be of type Individual, and its legal name and sold-to address must match identity documents. Individual validation involves the developer's government ID and mobile verification; the certificate exposes validated name/location fields, so the publisher is not an arbitrary `IRON SUN` brand name. Organizations in additional supported countries can apply using their legal business identity. Eligibility is conditional, not confirmation that this account will be approved. [Artifact Signing setup and individual validation](https://learn.microsoft.com/en-us/azure/artifact-signing/quickstart)

The service requires a **paid Azure subscription**; free, trial, and sponsored subscriptions are unsupported. The monthly charge is not prorated, and incomplete/failed identity onboarding can still leave a billable resource. The FAQ disallows custom CN/O values. Obtain payment approval and confirm the legal publisher choice before account creation. [Artifact Signing FAQ](https://learn.microsoft.com/en-us/azure/artifact-signing/faq)

Microsoft supports SignTool, PowerShell and GitHub Actions integration. Sign the launcher, installer and other executable artifacts with the validated identity; use timestamping and verify each final signature. Artifact Signing's short certificate validity makes timestamping essential. Then generate new immutable release identities and hashes, re-sign our existing release envelopes, and repeat download/install/startup checks. Signing changes binary bytes, so do not overwrite the already published unsigned release or pretend its antivirus evidence automatically covers the new artifact. [Microsoft signing integrations](https://learn.microsoft.com/en-us/azure/artifact-signing/how-to-signing-integrations)

**Recommendation:** use Artifact Signing Basic for the existing website/GitHub-download flow if the developer is eligible and approves the recurring cost plus identity validation. This is the smallest practical publisher-trust improvement for our established direct distribution. Initial reputation prompts can remain; retain honest messaging and validate actual first-run behavior. This recommendation is an inference from cost, eligibility, supported tools and our current distribution architecture, not a Microsoft guarantee.

## Free or alternative distribution

Microsoft Store **MSIX** distribution supplies Microsoft package signing after certification. Merely submitting our current EXE installer to the Store would not get it re-signed: the publisher must supply trusted Authenticode signatures for MSI/EXE submissions. Microsoft's signing comparison positions Artifact Signing as the lower-cost outside-Store option; traditional CA OV certificates and EV certificates also require validation and do not guarantee an initial warning-free experience. Its CA price ranges are general guidance, not vendor quotations. [Microsoft signing options](https://learn.microsoft.com/en-us/windows/apps/package-and-deploy/code-signing-options)

New individual Store developer registration is free through the supported `storedeveloper.microsoft.com` flow, subject to ID/selfie verification. This is not a promise that our product passes Store certification. [Microsoft free individual registration](https://learn.microsoft.com/en-us/windows/apps/publish/whats-new-individual-developer)

Our single-game downloader is **not automatically a free Store workaround**. Current Store policy 10.1.6 excludes a single game/franchise from the independent storefront exception; 10.2.5 requires game products and in-product offerings to install/update through the Store. The HTTPS EXE submission exception in 10.2.9 is for non-gaming products and requires signed, standalone installers. Packaging/distributing the game through the Store and redesigning updates could be legitimate, but needs explicit policy/architecture evaluation and certification. [Microsoft Store policies](https://learn.microsoft.com/en-us/windows/apps/publish/store-policies)

Directly hosted MSIX with an `.appinstaller` can support automatic updates, but requires a publicly trusted signing certificate for ordinary users. It does not turn self-signing into a frictionless public path. Changing hosting sites or offering a ZIP alone does not supply our executable with an Authenticode publisher identity (inference). [Microsoft distribution paths](https://learn.microsoft.com/en-us/windows/apps/package-and-deploy/choose-distribution-path)

## Source nuance and remaining uncertainty

The Azure product page mentions base SmartScreen reputation, while the current detailed Windows guidance and Artifact Signing FAQ state that signed files can still show prompts as reputation builds. Use the explicit operational guidance rather than interpreting marketing language as immediate clearance. The FAQ suggests submitting a persistently warned signed file for further review; the Windows guidance says there is no manual consumer reputation guarantee. Neither supports a promise that another submission will remove this warning. [Artifact Signing FAQ](https://learn.microsoft.com/en-us/azure/artifact-signing/faq), [SmartScreen reputation guidance](https://learn.microsoft.com/en-us/windows/apps/package-and-deploy/smartscreen-reputation)

No verified free direct-download public signing solution was identified for this private indie game. No exact time-to-reputation or warning-removal probability can be responsibly assigned. Publisher onboarding, approval, final paid quote and protected friend-PC first-run remain unperformed.
