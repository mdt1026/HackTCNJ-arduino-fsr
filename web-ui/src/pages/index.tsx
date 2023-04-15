import { type NextPage } from "next";
import Head from "next/head";

import MainScene from "components/MainScene";

const Home: NextPage = () => {
  return (
    <>
      <Head>
        <title>FSR Web Ui</title>
        <meta name="description" content="HackTCNJ Entry" />
        <link rel="icon" href="/favicon.ico" />
      </Head>

      <MainScene />
    </>
  );
};

export default Home;
